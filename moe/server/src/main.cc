/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/crtn/crt0.h>
#include <l4/util/util.h>
#include <l4/sigma0/sigma0.h>

#include <l4/sys/kip>
#include <l4/sys/utcb.h>
#include <l4/sys/debugger.h>
#include <l4/sys/scheduler>
#include <l4/sys/thread>
#include <l4/sys/cxx/ipc_server_loop>
#include <l4/re/error_helper>

#include <l4/cxx/exceptions>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>

#include <l4/util/mb_info.h>
#include <typeinfo>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "boot_fs.h"
#include "exception.h"
#include "globals.h"
#include "loader_elf.h"
#include "log.h"
#include "name_space.h"
#include "page_alloc.h"
#include "pages.h"
#include "vesa_fb.h"
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


static Dbg info(Dbg::Info);
static Dbg boot(Dbg::Boot);
static Dbg warn(Dbg::Warn);

static
l4_kernel_info_t const *map_kip()
{
  // map the KIP 1:1, because moe hast all memory 1:1 and the kip would
  // possibly overlap with 1:1 memory if we have lots of RAM.
  _current_kip = l4sigma0_map_kip(Sigma0_cap, 0, L4_WHOLE_ADDRESS_SPACE);

  if (!_current_kip)
    {
      Err(Err::Fatal).printf("could not map KIP\n");
      exit(1);
    }

  boot.printf("KIP @%p\n", kip());
  return kip();
}

static
char *my_cmdline()
{
  l4util_mb_info_t const *_mbi_ = (l4util_mb_info_t const *)(unsigned long)kip()->user_ptr;
  boot.printf("mbi @%p\n", _mbi_);
  l4util_mb_mod_t const *modules = (l4util_mb_mod_t const *)(unsigned long)_mbi_->mods_addr;
  unsigned num_modules = _mbi_->mods_count;
  char *cmdline = 0;

  for (unsigned mod = 0; mod < num_modules; ++mod)
    if (strstr((char const *)(unsigned long)modules[mod].cmdline, PROG))
      {
        cmdline = (char *)(unsigned long)modules[mod].cmdline;
        break;
      }

  if (!cmdline)
    cmdline = (char *)(unsigned long)_mbi_->cmdline;

  static char default_cmdline[] = "";

  if (!cmdline)
    {
      Dbg(Dbg::Warn).printf("No command line found, using default!\n");
      cmdline = default_cmdline;
    }

  return cmdline;
}



static void find_memory()
{
  using Moe::Pages::pages;
  l4_addr_t addr;
  l4_addr_t min_addr = ~0UL;
  l4_addr_t max_addr = 0;

  for (unsigned order = 30 /*1G*/; order >= L4_LOG2_PAGESIZE; --order)
    {
      while (!l4sigma0_map_anypage(Sigma0_cap, 0, L4_WHOLE_ADDRESS_SPACE,
                                   &addr, order))
        {
          unsigned long size = 1UL << order;

          if (addr == 0)
            {
              addr = L4_PAGESIZE;
              size -= L4_PAGESIZE;
              if (!size)
                continue;
            }

          if (addr < min_addr) min_addr = addr;
          if (addr + size > max_addr) max_addr = addr + size;

          Single_page_alloc_base::_free((void*)addr, size, true);
        }
    }

  info.printf("found %ld KByte free memory\n",
              Single_page_alloc_base::_avail() / 1024);

  // adjust min_addr and max_addr to also contain boot modules
  for (auto const &md: L4::Kip::Mem_desc::all(kip()))
    {
      if (md.is_virtual())
        continue;

      L4::Kip::Mem_desc::Mem_type type = md.type();
      unsigned long end = l4_round_page(md.end());
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

  pages = (__typeof(pages))Single_page_alloc_base::_alloc(sizeof(*pages) * total_pages);

  if (pages == 0)
    {
      Err(Err::Fatal).printf("could not allocate page array, halt\n");
      exit(128);
    }

  memset(pages, 0, sizeof(*pages) * total_pages);

  Moe::Pages::base_addr = min_addr;
  Moe::Pages::max_addr  = max_addr;

  info.printf("found RAM from %lx to %lx\n",
              min_addr, max_addr);
  info.printf("allocated %ld KByte for the page array @%p\n",
              sizeof(*pages) * total_pages / 1024, pages);
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
                                     L4_PAGESIZE, 0);
  if (!kip_ds)
    {
      Err(Err::Fatal).printf("could not allocate dataspace for KIP!\n");
      exit(1);
    }

  object_pool.cap_alloc()->alloc(kip_ds);
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
    l4_utcb_br_u(utcb)->br[1] = 0;
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

    dbg.printf("tag=%lx (proto=%lx) obj=%lx", tag.raw,
               tag.label(), obj);

    if (tag.is_exception())
      {
        dbg.cprintf("\n");
        Dbg(Dbg::Exceptions).printf("unhandled exception...\n");
        return l4_msgtag(-L4_ENOREPLY, 0, 0, 0);
      }
    else
      {
        // L4::cout << "ROOT: CALL(" << (void*)obj<< "): " << op << "...\n";
        o = r.find(obj & ~3UL);

        // L4::cout << "ROOT: obj=" << o << "\n";

        // enter_kdebug("a");

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

    Dbg(Dbg::Warn).printf("Invalid message (tag.label=%ld)\n", tag.label());
    return l4_msgtag(-L4_ENOSYS, 0, 0, 0);
  }

};

static cxx::String _init_prog = "rom/ned";

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
          warn.printf("ignore unknown argument for %.*s: '%.*s'\n",
                      opt.len(), opt.start(), a.len(), a.start());

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

static void hdl_ldr_flags(cxx::String const &args)
{
  unsigned long lvl = parse_flags(args, ldr_flag_bits, "--ldr-flags");
  Moe::ldr_flags = lvl;
}



static Get_opt const _options[] = {
      {"--debug=",     hdl_debug },
      {"--init=",      hdl_init },
      {"--l4re-dbg=",  hdl_l4re_dbg },
      {"--ldr-flags=", hdl_ldr_flags },
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

  warn.printf("unknown command-line option '%.*s'\n", o.len(), o.start());
}

static void
parse_option(cxx::String const &o)
{
  if (o.len() < 2)
    {
      warn.printf("empty command-line option '%.*s'\n", o.len(), o.start());
      return;
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
          warn.printf("unkown command-line option '%c'\n", o[s]);
          break;
        }
    }
}

static Elf_loader elf_loader;

static L4::Server<Loop_hooks> server(l4_utcb());


static void init_env()
{
  // setup log capability in the global env, so that the libc backend can use
  // L4::Env::env()->log() to send logoutput to
  l4re_global_env = reinterpret_cast<l4re_env_t*>(&my_env);
  my_env.factory(L4_BASE_FACTORY_CAP);
  my_env.log(L4_BASE_LOG_CAP);
  my_env.scheduler(L4_BASE_SCHEDULER_CAP);
}

static __attribute__((used, section(".preinit_array")))
   const void *pre_init_env = (void *)init_env;

static void init_emergency_memory()
{
  // populate the page allocator with a few pages of static memory to allow for
  // dynamic allocation of memory arena during static initialization of stdc++'s
  // emergency_pool in GCC versions 5 and newer
  static __attribute__((aligned(L4_PAGESIZE))) char buf[3 * L4_PAGESIZE];
  Single_page_alloc_base::_free(buf, sizeof(buf), true);
}

static __attribute__((used, section(".preinit_array")))
   const void *pre_init_emergency_memory = (void *)init_emergency_memory;

int main(int argc, char**argv)
{
  (void)argc; (void)argv;
  Dbg::set_level(Dbg::Info | Dbg::Warn);
  //Dbg::set_level(Dbg::Info | Dbg::Warn | Dbg::Boot);

  info.printf("Hello world\n");

  L4Re::Env::env()->scheduler()
    ->run_thread(L4::Cap<L4::Thread>(L4_BASE_THREAD_CAP), l4_sched_param(0xff));

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
      find_memory();
      init_virt_limits();
#if 0
      extern unsigned page_alloc_debug;
      page_alloc_debug = 1;
#endif
      Moe::Boot_fs::init_stage2();
      init_vesa_fb((l4util_mb_info_t *)kip()->user_ptr);

      root_name_space_obj = object_pool.cap_alloc()->alloc(root_name_space());

      init_kip_ds();

      object_pool.cap_alloc()->alloc(Allocator::root_allocator());

      l4_debugger_set_object_name(L4_BASE_TASK_CAP,   "moe");
      l4_debugger_set_object_name(L4_BASE_THREAD_CAP, "moe");
      l4_debugger_set_object_name(L4_BASE_PAGER_CAP,  "moe->s0");

      root_name_space()->register_obj("log", Entry::F_rw, L4_BASE_LOG_CAP);
      root_name_space()->register_obj("icu", Entry::F_rw, L4_BASE_ICU_CAP);
      if (L4::Cap<void>(L4_BASE_IOMMU_CAP).validate().label())
        root_name_space()->register_obj("iommu", Entry::F_rw, L4_BASE_IOMMU_CAP);
      root_name_space()->register_obj("sigma0", Entry::F_trusted | Entry::F_rw, L4_BASE_PAGER_CAP);
      root_name_space()->register_obj("mem", Entry::F_trusted | Entry::F_rw, Allocator::root_allocator());
      root_name_space()->register_obj("jdb", Entry::F_trusted | Entry::F_rw, L4_BASE_DEBUGGER_CAP);

      char *cmdline = my_cmdline();

      info.printf("cmdline: %s\n", cmdline);

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
              elf_loader.start(_init_prog, cxx::String(a.first.start(),
                                                       a.second.end()));
              break;
            }

          if (a.first == "--")
            {
              elf_loader.start(_init_prog, a.second);
              break;
            }

          parse_option(a.first);
        }

      if (a.first.empty())
        elf_loader.start(_init_prog, cxx::String(""));

      // dump name space information
      if (boot.is_active())
        {
          boot.printf("dump of root name space:\n");
          root_name_space()->dump(1);
        }

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
