/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/bid_config.h>
#include <l4/util/kip.h>
#include <l4/util/util.h>
#include <l4/sigma0/sigma0.h>

#include <l4/sys/assert.h>
#include <l4/sys/kip>
#include <l4/sys/utcb.h>
#include <l4/sys/debugger.h>
#include <l4/sys/scheduler>
#include <l4/sys/thread>
#include <l4/sys/cxx/ipc_server_loop>
#include <l4/re/error_helper>
#include <l4/util/printf_helpers.h>

#include <l4/cxx/exceptions>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>
#include <l4/cxx/static_container>

#include <l4/util/l4mod.h>
#include <typeinfo>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "dma_space.h"
#include "boot_fs.h"
#include "mem_region_list.h"
#include "factory_config.h"
#include "globals.h"
#include "loader_elf.h"
#include "log.h"
#include "name_space.h"
#include "page_alloc.h"
#include "pages.h"
#include "boot_fb.h"
#include "dataspace_static.h"
#include "debug.h"
#include "args.h"

#include <l4/re/env>

using Moe::Entry;

static L4Re::Env my_env;

// Implementation
extern "C" void _exit(int status);

void _exit(int status)
{
  L4::cout << "MOE: is terminating with " << status << ", very bad......\n";
  l4_sleep_forever();
}

unsigned Moe::l4re_dbg = Dbg::Warn;
unsigned Moe::ldr_flags;

Moe::Factory_config Moe::root_factory_config;

static Dbg info(Dbg::Info);
static Dbg boot(Dbg::Boot);
static Dbg warn(Dbg::Warn);

#ifdef CONFIG_BID_PIE
static inline unsigned long elf_machine_load_address()
{
  extern const char __ehdr_start __attribute__((visibility ("hidden")));
  return reinterpret_cast<unsigned long>(&__ehdr_start);
}
#else
static inline unsigned long elf_machine_load_address()
{ return 0; }
#endif

static
l4_kernel_info_t const *map_kip()
{
  // map the KIP 1:1, because moe has all memory 1:1 and the kip would
  // possibly overlap with 1:1 memory if we have lots of RAM.
  _current_kip = l4sigma0_map_kip(Sigma0_cap, 0, L4_WHOLE_ADDRESS_SPACE);

  if (!_current_kip)
    {
      Err(Err::Fatal).printf("could not map KIP\n");
      exit(1);
    }

  boot.printf("KIP @%p\n", kip());
  l4_global_kip = _current_kip;
  return kip();
}

static
char *my_cmdline()
{
  auto *_mbi_ = reinterpret_cast<l4util_l4mod_info const *>(kip()->user_ptr);
  boot.printf("mbi @%p\n", _mbi_);
  auto  *modules = reinterpret_cast<l4util_l4mod_mod const *>(_mbi_->mods_addr);
  unsigned num_modules = _mbi_->mods_count;
  char *cmdline = 0;

  for (unsigned mod = 0; mod < num_modules; ++mod)
    if ((modules[mod].flags & L4util_l4mod_mod_flag_mask) == L4util_l4mod_mod_flag_roottask)
      {
        cmdline = reinterpret_cast<char *>(
                    static_cast<unsigned long>(modules[mod].cmdline));
        break;
      }

  if (!cmdline)
    cmdline = reinterpret_cast<char *>(
                static_cast<unsigned long>(_mbi_->cmdline));

  static char default_cmdline[] = "";

  if (!cmdline)
    {
      Dbg(Dbg::Warn).printf("No command line found, using default!\n");
      cmdline = default_cmdline;
    }

  return cmdline;
}


l4_size_t Moe::Phys_limit::avail_ram;

static void find_memory()
{
  Single_page_alloc_base::can_free =
    l4util_kip_kernel_has_feature(kip(), "mapdb");
  if (!Single_page_alloc_base::can_free)
    info.printf("Fiasco mapdb not available! Memory cannot be given back!\n");

  // List of free regions (allocated on stack).
  Moe::Mem_region_list<Moe::Mem_range, 100> free_map;

  // First scan and map all available memory, and populate the free list.
  l4_addr_t addr;
  l4_addr_t min_addr = Moe::Max_phys_addr;
  l4_addr_t max_addr = 0;
  for (unsigned order = Moe::Max_phys_page_order; order >= L4_LOG2_PAGESIZE; --order)
    {
      while (!l4sigma0_map_anypage(Sigma0_cap, 0, L4_WHOLE_ADDRESS_SPACE,
                                   &addr, order))
        {
          unsigned long size = 1UL << order;

          // Moe identity maps physical memory into its virtual address space.
          // To not clash with nullptr, physical memory at address zero must not
          // be used.
          if (addr == 0)
            {
              addr = L4_PAGESIZE;
              size -= L4_PAGESIZE;
              if (!size)
                continue;
            }

          if (addr < min_addr)
            min_addr = addr;
          if (addr + size > max_addr)
            max_addr = addr + size;

          Moe::Mem_range range{addr, addr + size - 1};
          if (free_map.add(range))
            continue; // success

          // TODO: Initial stack allocated free map is exhausted.
          //       Need to allocate temporary free map from the free memory we already discovered.
          //       Must be a region specified as untyped in Moe::root_factory_config.
          Err(Err::Normal).printf("free map exhausted\n");
          // For now just ignore the remaining memory.
          break;
        }
      if (free_map.full())
        break;
    }

  info.printf("Free map:-----------------------\n");
  free_map.dump(info);

  // Now find the optimal place for buddy allocator free bits.
  unsigned long metadata_bytes =
    Single_page_alloc_base::_metadata_bytes(min_addr, max_addr);
  info.printf("Allocator free map:-------------\n");
  info.printf("Required bytes: %#lx\n", metadata_bytes);

  Moe::Mem_range metadata_region;
  // Find smallest region to avoid destroying precious large aligned ram.
  for (auto const &free_region : free_map)
    {
      if (free_region.size() < metadata_bytes)
        continue;

      // Only parts of the region in untyped memory are eligible.
      for (auto const &untyped_region : Moe::root_factory_config.untyped_regions())
        {
          auto intersect = free_region.intersect(untyped_region.range);
          if (!intersect.valid() || intersect.size() < metadata_bytes)
            continue;

          if (metadata_region.valid() && metadata_region.size() <= intersect.size())
            continue;

          metadata_region = intersect;
        }
    }

  if (!metadata_region.valid())
    {
      Err(Err::Fatal).printf("no suitable region for alloc metadata\n");
      exit(128);
    }

  // We allocate the metadata at the start of the selected region.
  metadata_region.end = metadata_region.start + metadata_bytes - 1;

  info.printf("Allocator metadata region:\n");
  metadata_region.dump(info);

  Single_page_alloc_base::_init(
    min_addr, max_addr, reinterpret_cast<unsigned char *>(metadata_region.start),
    metadata_bytes);

  // Finally add the memory to the allocator
  for (auto const &region : free_map)
    {
      // Carve out metadata region.
      if (region.contains(metadata_region))
        {
          if (metadata_region.start > region.start)
            Single_page_alloc_base::_add_mem(
              reinterpret_cast<void *>(region.start),
              metadata_region.start - region.start);

          if (metadata_region.end < region.end)
            Single_page_alloc_base::_add_mem(
              reinterpret_cast<void *>(metadata_region.end + 1),
              region.end - metadata_region.end);
        }
      else
        Single_page_alloc_base::_add_mem(reinterpret_cast<void *>(region.start),
                                         region.size());
    }

  Moe::Phys_limit::avail_ram = Single_page_alloc_base::_avail();
  char str[64];
  l4util_human_readable_size(str, sizeof(str), Single_page_alloc_base::_avail());
  info.printf("found %s RAM in the area %08lx..%08lx\n", str, min_addr, max_addr);

#ifdef CONFIG_MMU
  // adjust min_addr and max_addr to also contain boot modules
  for (auto const &md: L4::Kip::Mem_desc::all(kip()))
    {
      if (md.is_virtual())
        continue;

      L4::Kip::Mem_desc::Mem_type type = md.type();
      unsigned long end = l4_round_page(md.end() + 1);
      unsigned long start = l4_trunc_page(md.start());
      switch (type)
        {
        case L4::Kip::Mem_desc::Bootloader:
          if (start < min_addr)
            min_addr = start;
          if (end > max_addr)
            max_addr = end;
          break;
        case L4::Kip::Mem_desc::Conventional:
        case L4::Kip::Mem_desc::Reserved:
        case L4::Kip::Mem_desc::Dedicated:
        case L4::Kip::Mem_desc::Arch:
        case L4::Kip::Mem_desc::Shared:
        default:
          break;
        }
    }

  assert(max_addr > min_addr);
  l4_addr_t total_pages = (max_addr - min_addr) >> L4_PAGESHIFT;

  assert(total_pages);

  using Moe::Pages::pages;
  pages = static_cast<decltype(pages)>(Single_page_alloc_base::_alloc(
    sizeof(*pages) * total_pages, alignof(decltype(*pages))));

  if (pages == 0)
    {
      Err(Err::Fatal).printf("could not allocate page array, halt\n");
      exit(128);
    }

  memset(pages, 0, sizeof(*pages) * total_pages);

  Moe::Pages::base_addr = min_addr;
  Moe::Pages::max_addr  = max_addr;

  l4util_human_readable_size(str, sizeof(str), sizeof(*pages) * total_pages);
  info.printf("allocated %s for the page array @%p\n", str, pages);
#endif
}

l4_addr_t Moe::Virt_limit::start;
l4_addr_t Moe::Virt_limit::end;

static void
init_virt_limits()
{
  for (auto const &m: L4::Kip::Mem_desc::all(kip()))
    {
      if (m.type() != L4::Kip::Mem_desc::Conventional || !m.is_virtual())
        continue;

      Moe::Virt_limit::start = m.start();
      Moe::Virt_limit::end = m.end();
    }

  info.printf("virtual user address space [%lx-%lx]\n",
              Moe::Virt_limit::start,
              Moe::Virt_limit::end);
}

static void
init_utcb()
{
  l4_utcb_t *u = l4_utcb();
  boot.printf("UTCB @%p\n", u);
  if (!u)
    abort();
}

static void
init_kip_ds()
{
  kip_ds = new Moe::Dataspace_static(const_cast<l4_kernel_info_t *>(kip()),
                                     L4_PAGESIZE, L4Re::Dataspace::F::RX);
  if (!kip_ds)
    {
      Err(Err::Fatal).printf("could not allocate dataspace for KIP!\n");
      exit(1);
    }

  object_pool.cap_alloc()->alloc(kip_ds, "moe-ds-kip");
}

static L4::Cap<void>
new_sigma0_cap()
{
  L4::Cap<void> new_sigma0_cap = object_pool.cap_alloc()->alloc();

  L4Re::chksys(
    L4::Cap<L4::Factory>(Sigma0_cap)->create(new_sigma0_cap, L4_PROTO_SIGMA0),
    "Create new sigma0 cap for the initial task.");

  return new_sigma0_cap;
}

class Loop_hooks :
  public L4::Ipc_svr::Ignore_errors,
  public L4::Ipc_svr::Default_timeout,
  public L4::Ipc_svr::Compound_reply
{
public:
  static void setup_wait(l4_utcb_t *utcb, L4::Ipc_svr::Reply_mode)
  {
    l4_utcb_br_u(utcb)->br[0] = L4::Ipc::Small_buf(Rcv_cap << L4_CAP_SHIFT,
                                                   L4_RCV_ITEM_LOCAL_ID).raw();
    l4_utcb_br_u(utcb)->br[1] = L4::Ipc::Small_buf(Rcv_cap2 << L4_CAP_SHIFT,
                                                   L4_RCV_ITEM_LOCAL_ID).raw();
    l4_utcb_br_u(utcb)->br[2] = 0;
    l4_utcb_br_u(utcb)->bdr = 0;
  }
};

template< typename Reg >
class My_dispatcher
{
private:
  Reg r;

public:
  l4_msgtag_t dispatch(l4_msgtag_t tag, l4_umword_t obj, l4_utcb_t *utcb)
  {
    typename Reg::Value *o = 0;

#if 0
    l4_utcb_t *u = l4_utcb();
    L4::cout << L4::hex << "UTCB: " << u->values[0]
      << "(" << op << "):" << u->values[1]
      << "(" << obj << ")\n";
#endif

    Dbg dbg(Dbg::Server);

    dbg.printf("tag=%lx (proto=%ld) obj=%lx", tag.raw, tag.label(), obj);

    o = r.find(obj & ~3UL);

    if (!o)
      {
        dbg.cprintf(": invalid object\n");
        return l4_msgtag(-L4_ENOENT, 0, 0, 0);
      }

    dbg.cprintf(": object is a %s\n", typeid(*o).name());
    try
      {
        l4_msgtag_t res = o->dispatch(tag, obj, utcb);
        dbg.printf("reply = %ld\n", res.label());
        return res;
      }
    catch (L4::Runtime_error &e)
      {
        int res = e.err_no();
        dbg.printf("reply(exception) = %d\n", res);
        return l4_msgtag(res, 0, 0, 0);
      }
  }

};

static cxx::String _init_prog = "rom/ned";
static cxx::String _root_factory_config;
static cxx::String _root_factory_brk;

struct Get_opt
{
  char const *tag;
  void (*hdl)(cxx::String const &);
};

struct Dbg_bits { char const *tag; unsigned long bits; };
static Dbg_bits const dbb[] =
  {{"info",       Dbg::Info},
   {"nfo",        Dbg::Info},
   {"warn",       Dbg::Warn},
   {"boot",       Dbg::Boot},
   {"server",     Dbg::Server},
   {"svr",        Dbg::Server},
   {"exceptions", Dbg::Exceptions},
   {"exc",        Dbg::Exceptions},
   //{"POSIX",      0x40},
   //{"ldso",       0x40},
   {"loader",     Dbg::Loader},
   {"ldr",        Dbg::Loader},
   {"ns",         Dbg::Name_space},
   {"all",        ~0UL},
   {0, 0}};

static Dbg_bits const ldr_flag_bits[] =
  {{"pre_alloc",    L4RE_AUX_LDR_FLAG_EAGER_MAP},
   {"eager_map",    L4RE_AUX_LDR_FLAG_EAGER_MAP},
   {"all_segs_cow", L4RE_AUX_LDR_FLAG_ALL_SEGS_COW},
   {"pinned_segs",  L4RE_AUX_LDR_FLAG_PINNED_SEGS},
   {"exit",  0x10},
   {0, 0}};

static unsigned long parse_flags(cxx::String const &_args, Dbg_bits const *dbb,
                                 cxx::String const &opt)
{
  cxx::String args = _args;
  unsigned long lvl = 0;
  for (;;)
    {
      cxx::String::Index c = args.find(",|+");
      cxx::String a = args.head(c);

      if (a.empty())
        break;

      args = args.substr(c+1);
      Dbg_bits const *b;

      for (b = &dbb[0]; b->tag; ++b)
        {
          if (a == b->tag)
            {
              lvl |= b->bits;
              break;
            }
        }

      if (!b->tag)
        {
          Err(Err::Fatal).printf("Unknown argument for %.*s: '%.*s'\n",
                                 opt.len(), opt.start(), a.len(), a.start());
          exit(1);
        }
    }
  return lvl;
}

static void hdl_debug(cxx::String const &args)
{
  Dbg::set_level(parse_flags(args, dbb, "--debug"));
}

static void hdl_init(cxx::String const &args)
{
  _init_prog = args;
}

static void hdl_l4re_dbg(cxx::String const &args)
{
  unsigned long lvl = parse_flags(args, dbb, "--l4re-dbg");
  Moe::l4re_dbg = lvl;
}

static void hdl_root_factory(cxx::String const &_args)
{
  // Fail if the --root-factory option is specified multiple times, to prevent
  // accidental misconfiguration.
  if (!_root_factory_config.empty())
    {
      Err(Err::Fatal)
        .printf("The --root-factory option was specified multiple times.\n");
      exit(1);
    }
  _root_factory_config = _args;
}

static void hdl_ldr_flags(cxx::String const &args)
{
  unsigned long lvl = parse_flags(args, ldr_flag_bits, "--ldr-flags");
  Moe::ldr_flags = lvl;
}

#ifndef CONFIG_MMU
static void hdl_brk(cxx::String const &args)
{
  _root_factory_brk = args;
}
#endif


static Get_opt const _options[] = {
      {"--debug=",     hdl_debug },
      {"--init=",      hdl_init },
      {"--root-factory=", hdl_root_factory },
      {"--l4re-dbg=",  hdl_l4re_dbg },
      {"--ldr-flags=", hdl_ldr_flags },
#ifndef CONFIG_MMU
      {"--brk=",       hdl_brk },
#endif
      {0, 0}
};

static void
parse_long_option(cxx::String const &o)
{

  for (Get_opt const *opt = _options; opt->tag; ++opt)
    {
      if (o.starts_with(opt->tag))
        {
          opt->hdl(o.substr(strlen(opt->tag)));
          return;
        }
    }

  Err(Err::Fatal).printf("Unknown command-line option '%.*s'\n",
                         o.len(), o.start());
  exit(1);
}

static void
parse_option(cxx::String const &o)
{
  if (o.len() < 2)
    {
      Err(Err::Fatal).printf("Empty command-line option '%.*s'\n",
                             o.len(), o.start());
      exit(1);
    }

  if (o[1] == '-')
    {
      parse_long_option(o);
      return;
    }

  for (cxx::String::Index s = o.start() + 1; !o.eof(s); ++s)
    {
      switch (o[s])
        {
        default:
          Err(Err::Fatal).printf("Unknown command-line option '%c'\n", o[s]);
          exit(1);
        }
    }
}

static cxx::String parse_cmdline()
{
  char *cmdline = my_cmdline();
  info.printf("cmdline: %s\n", cmdline);

  cxx::String init_args("");
  bool skip_argv0 = true;
  cxx::Pair<cxx::String, cxx::String> a;
  for (a = next_arg(cmdline); !a.first.empty(); a = next_arg(a.second))
    {
      if (skip_argv0)
        {
          skip_argv0 = false;
          continue;
        }

      if (a.first[0] != '-') // not an option start init
        {
          init_args = cxx::String(a.first.start(), a.second.end());
          break;
        }

      if (a.first == "--")
        {
          init_args = a.second;
          break;
        }

      parse_option(a.first);
    }
  return init_args;
}

static void init_root_factory_config()
{
  if (_root_factory_config.empty())
    {
      l4_addr_t physmin = 0;
      if (!_root_factory_brk.empty() && _root_factory_brk.from_hex(&physmin) <= 0)
        {
          Err(Err::Fatal).printf("Invalid --brk option: '%.*s'\n",
                                 _root_factory_brk.len(),
                                 _root_factory_brk.start());
          exit(1);
        }

      // Use default values if no --root-factory config is provided.
      Moe::Mem_range all_mem{physmin, Moe::Max_phys_addr};
      if (!Moe::root_factory_config.regions.add(Moe::Mem_region::untyped(all_mem)))
        abort(); // should never happen
      Moe::root_factory_config.permissions = Moe::Factory_config::Scheduler_proxy;
      return;
    }

  if (!_root_factory_brk.empty())
    {
      Err(Err::Fatal).printf(
        "The options --brk and --root-factory are are mutually exclusive.\n");
      exit(1);
    }

  cxx::String args = _root_factory_config;
  for (;;)
  {
    cxx::String::Index delimiter = args.find(',');
    cxx::String arg = args.head(delimiter);
    if (arg.empty())
      break;

    args = args.substr(delimiter + 1);
    if (!Moe::parse_factory_config(arg, &Moe::root_factory_config))
      {
        Err(Err::Fatal).printf("Unknown argument for --root-factory: '%.*s'\n",
                               arg.len(), arg.start());
        exit(1);
      }
  }

  // Similar to sub-factory creation, if no region was specified at all, use
  // all memory as untyped memory.
  if (Moe::root_factory_config.regions.empty())
    {
      Moe::Mem_range all_mem{0, Moe::Max_phys_addr};
      if (!Moe::root_factory_config.regions.add(Moe::Mem_region::untyped(all_mem)))
        abort(); // should never happen
    }

  if (Moe::root_factory_config.untyped_regions().empty())
    {
      Err(Err::Fatal).printf(
        "The --root-factory option did not specify any untyped regions.\n");
      exit(1);
    }
}

static void init_default_mem_cfg()
{
  // Moe internal allocations must only come from memory tagged as untyped in
  // the root factory config.
  Single_page_alloc_base::default_mem_cfg.regions =
    Moe::root_factory_config.untyped_regions();
}

static cxx::Static_container<Moe::Dma_space_mgr> dma_space_mgr;
static Elf_loader elf_loader;
static L4::Server<Loop_hooks> server;


static void init_env()
{
  // setup log capability in the global env, so that the libc backend can use
  // L4::Env::env()->log() to send logoutput to
  l4re_global_env = reinterpret_cast<l4re_env_t*>(&my_env);
  my_env.factory(L4_BASE_FACTORY_CAP);
  L4::Cap<L4Re::Log> klog(L4_BASE_LOG_CAP);
  my_env.log(klog.validate().label() > 0 ? klog : L4::Cap<L4Re::Log>());
  my_env.scheduler(L4_BASE_SCHEDULER_CAP);
}

static __attribute__((used, section(".preinit_array")))
   const void *pre_init_env = reinterpret_cast<void *>(init_env);

int main(int /* argc */, char** /* argv */)
{
  Dbg::set_level(Dbg::Info | Dbg::Warn);
  //Dbg::set_level(Dbg::Info | Dbg::Warn | Dbg::Boot);

  info.printf("Hello world\n");

  l4_msgtag_t res = L4Re::Env::env()->scheduler()
    ->run_thread(L4::Cap<L4::Thread>(L4_BASE_THREAD_CAP), l4_sched_param(0xff));
  if (l4_error(res) < 0)
    warn.printf("could not set scheduling priority\n");

#if 0 // map RO stuff RO, we'll see write pagefaults if someone write to our
      // RO segments later on
  extern char _stext[];
  extern char _etext[];

  for (unsigned long i = (unsigned long)_stext; i < (unsigned long)_etext;
       i+= L4_PAGESIZE)
    {
      l4_task_unmap(l4_utcb(), L4_BASE_TASK_CAP,
                    l4_fpage(i, L4_PAGESHIFT, L4_FPAGE_W), L4_FP_ALL_SPACES);
    }
#endif

  try
    {
      map_kip();
      init_utcb();
      Moe::Boot_fs::init_stage1();

      cxx::String init_args = parse_cmdline();

      init_root_factory_config();
      info.printf("Root factory:-------------------\n");
      Moe::root_factory_config.dump(info);

      init_default_mem_cfg();

      find_memory();
      init_virt_limits();

#if 0
      extern unsigned page_alloc_debug;
      page_alloc_debug = 1;
#endif
      Moe::Boot_fs::init_stage2();
      init_boot_fb((l4util_l4mod_info *)kip()->user_ptr);

      root_name_space_obj = object_pool.cap_alloc()->alloc(root_name_space(),
                                                           "moe-root-ns");

      init_kip_ds();

      object_pool.cap_alloc()->alloc(Allocator::root_allocator(), "moe-rootalloc");

      l4_debugger_set_object_name(L4_BASE_TASK_CAP,   "moe");
      l4_debugger_set_object_name(L4_BASE_THREAD_CAP, "moe");
      l4_debugger_set_object_name(L4_BASE_PAGER_CAP,  "moe->s0");
      l4_debugger_add_image_info(L4_BASE_TASK_CAP, elf_machine_load_address(),
                                 "moe");

      root_name_space()->register_obj("log", Entry::F_rw, L4_BASE_LOG_CAP);
      root_name_space()->register_obj("icu", Entry::F_rw, L4_BASE_ICU_CAP);

      dma_space_mgr.construct(root_name_space(), "dma_mgr");

      if (L4::Cap<void>(L4_BASE_IOMMU_CAP).validate().label())
        root_name_space()->register_obj("iommu", Entry::F_rw, L4_BASE_IOMMU_CAP);
      if (L4::Cap<void>(L4_BASE_ARM_SMCCC_CAP).validate().label())
        root_name_space()->register_obj("arm_smc", Entry::F_rw,
                                        L4_BASE_ARM_SMCCC_CAP);
      root_name_space()->register_obj("sigma0", Entry::F_trusted | Entry::F_rw,
                                      new_sigma0_cap());
      root_name_space()->register_obj("mem", Entry::F_trusted | Entry::F_rw,
                                      Allocator::root_allocator());
      if (L4::Cap<void>(L4_BASE_DEBUGGER_CAP).validate().label())
        root_name_space()->register_obj("jdb", Entry::F_trusted | Entry::F_rw,
                                        L4_BASE_DEBUGGER_CAP);
      root_name_space()->register_obj("kip", Entry::F_rw, kip_ds->obj_cap());

      // dump name space information
      if (boot.is_active())
        {
          boot.printf("dump of root name space:\n");
          root_name_space()->dump(1);
        }

      elf_loader.start(_init_prog, init_args);

      // we handle our exceptions ourselves
      server.loop_noexc(My_dispatcher<L4::Basic_registry>());
    }
  catch (L4::Out_of_memory const &e)
    {
      Err(Err::Fatal).printf("Memory exhausted and not handled\n");
      L4::cerr << "FATAL exception in MOE:\n"
               << e << "... terminated\n";
    }
  catch (L4::Runtime_error const &e)
    {
      L4::cerr << "FATAL exception in MOE:\n"
               << e << "... terminated\n";
      abort();
    }
  return 0;
}
