/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/util/l4mod.h>
#include <l4/util/util.h>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>
#include <l4/sigma0/sigma0.h>
#include <l4/util/splitlog2.h>

#include "boot_fs.h"
#include "dataspace_static.h"
#include "page_alloc.h"
#include "globals.h"
#include "name_space.h"
#include "debug.h"
#include "args.h"

#include <cstring>
#include <cstdlib>

static Dbg dbg(Dbg::Boot_fs, "fs");

#if 0
#include <cstdio>
static void dump_mb_module(l4util_l4mod_mod const *mod)
{
  printf("  cmdline: '%s'\n"
         "    range: [%08llx; %08llx)\n",
         (char const *)mod->cmdline, mod->mod_start, mod->mod_end);
}

static void dump_mbi(l4util_l4mod_info const* mbi)
{
  printf("MBI Version: %08llx\n", mbi->flags);
  printf("cmdline: '%s'\n", (char const *)mbi->cmdline);
  l4util_l4mod_mod const *modules = (l4util_l4mod_mod const *)mbi->mods_addr;
  for (unsigned i = 0; i < mbi->mods_count; ++i)
    dump_mb_module(modules +i);
}
#endif

static inline cxx::String cmdline_to_name(char const *cmdl, cxx::String *opts)
{
  int len = strlen(cmdl);
  int i;
  for (i = 0; i < len; ++i)
    {
      if (i > 0 && cmdl[i] == ' ' && cmdl[i-1] != '\\')
        break;
    }

  int s;
  for (s = i - 1; s >= 0; --s)
    {
      if (cmdl[s] == '/')
        break;
    }

  ++s;

  *opts = cxx::String(cmdl + i, len - i);

  return cxx::String(cmdl + s, i - s);
}

static bool options_contains(cxx::String const &opts, cxx::String const &opt)
{
  cxx::Pair<cxx::String, cxx::String> a;
  for (a = next_arg(opts); !a.first.empty(); a = next_arg(a.second))
    if (opt == a.first)
      return true;

  return false;
}

void
Moe::Boot_fs::init_stage1()
{
  l4util_l4mod_info const *mbi = (l4util_l4mod_info const *)kip()->user_ptr;

  l4_touch_ro(mbi,10);
}

static long
s0_request_ram(l4_addr_t s, l4_addr_t, int order)
{
  l4_msg_regs_t *m = l4_utcb_mr();
  l4_buf_regs_t *b = l4_utcb_br();
  l4_msgtag_t tag = l4_msgtag(L4_PROTO_SIGMA0, 2, 0, 0);
  m->mr[0] = SIGMA0_REQ_FPAGE_RAM;
  m->mr[1] = l4_fpage(s, order, L4_FPAGE_RWX).raw;

  b->bdr   = 0;
  b->br[0] = L4_ITEM_MAP;
  b->br[1] = l4_fpage(s, order, L4_FPAGE_RWX).raw;
  tag = l4_ipc_call(Sigma0_cap, l4_utcb(), tag, L4_IPC_NEVER);
  return 0;
}

class Dirinfo
{
public:
  void add(cxx::String const &name)
  {
    do
      {
        unsigned left = _space - _size;
        unsigned written = snprintf(_buf + _size, left, "%d:%.*s\n",
                                    name.len(), name.len(), name.start());
        if (written > left)
          {
            char *n = (char *)Single_page_alloc_base::_alloc(_space + L4_PAGESIZE,
                                                             L4_PAGESHIFT);
            memcpy(n, _buf, _space);
            Single_page_alloc_base::_free(_buf, _space, true);
            _buf = n;
            _space += L4_PAGESIZE;
          }
        else
          {
            _size += written;
            break;
          }
      }
    while (1);
  }

  bool empty() const
  { return _size == 0; }

  void create_ds_and_register(Moe::Name_space *ns)
  {
    if (empty())
      return;

    Moe::Dataspace_static *ds;
    ds = new Moe::Dataspace_static((void *)_buf, _size, L4Re::Dataspace::F::R);

    object_pool.cap_alloc()->alloc(ds);
    ns->register_obj(".dirinfo", 0, ds);
  }

private:
  unsigned _space = L4_PAGESIZE;
  unsigned _size = 0;
  char *_buf = (char *)Single_page_alloc_base::_alloc(_space, L4_PAGESHIFT);
};

void
Moe::Boot_fs::init_stage2()
{
  auto *rom_ns = Moe::Moe_alloc::allocator()->make_obj<Moe::Name_space>();
  L4::Cap<void> rom_ns_cap = object_pool.cap_alloc()->alloc(rom_ns);
  L4::cout << "MOE: rom name space cap -> " << rom_ns_cap << '\n';
  root_name_space()->register_obj("rom", 0, rom_ns);

  auto *rwfs_ns = Moe::Moe_alloc::allocator()->make_obj<Moe::Name_space>();
  L4::Cap<void> rwfs_ns_cap = object_pool.cap_alloc()->alloc(rwfs_ns);
  L4::cout << "MOE: rwfs name space cap -> " << rwfs_ns_cap << '\n';
  root_name_space()->register_obj("rwfs", 0, rwfs_ns);

  L4::Cap<void> object;
  l4util_l4mod_info const *mbi = (l4util_l4mod_info const *)kip()->user_ptr;

  //dump_mbi(mbi);

  Dirinfo dirinfo_ro, dirinfo_rw;

  l4util_l4mod_mod const *modules = (l4util_l4mod_mod const *)(unsigned long)mbi->mods_addr;
  unsigned num_modules = mbi->mods_count;

  l4_addr_t m_low = -1;
  l4_addr_t m_high = 0; // inclusive!
  for (unsigned mod = 3; mod < num_modules; ++mod)
    {
      l4_addr_t s = modules[mod].mod_start;
      if (s != m_high + 1 && m_low != (l4_addr_t)-1)
        {
          l4util_splitlog2_hdl(m_low, m_high, s0_request_ram);
          m_low = -1;
          m_high = 0;
        }

      if (m_low > s)
        m_low = s;

      //l4_addr_t end = l4_round_page(modules[mod].mod_end);
      l4_addr_t end = modules[mod].mod_end;

      if (m_high < l4_round_page(end) - 1)
        m_high = l4_round_page(end) - 1;

      cxx::String opts;
      cxx::String name = cmdline_to_name((char const *)(unsigned long)modules[mod].cmdline, &opts);
      Dataspace::Flags flags = Dataspace::Flags(Dataspace::Cow_enabled) | L4Re::Dataspace::F::RX;
      if (options_contains(opts, cxx::String(":rw")))
        flags = L4Re::Dataspace::F::RWX;

      Moe::Dataspace_static *rf;
      rf = new Moe::Dataspace_static((void*)(unsigned long)modules[mod].mod_start,
                                     end - modules[mod].mod_start, flags);
      object = object_pool.cap_alloc()->alloc(rf);
      if (flags.w())
        {
          rwfs_ns->register_obj(name, Entry::F_rw, rf);
          dirinfo_rw.add(name);
        }
      else
        {
          rom_ns->register_obj(name, 0, rf);
          dirinfo_ro.add(name);
        }

      L4::cout << "  BOOTFS: [" << (void*)(unsigned long)modules[mod].mod_start << "-"
               << (void*)end << "] " << object << " "
               << name << "\n";
    }

  if (m_low != (l4_addr_t)-1)
    l4util_splitlog2_hdl(m_low, m_high, s0_request_ram);

  dirinfo_ro.create_ds_and_register(rom_ns);
  dirinfo_rw.create_ds_and_register(rwfs_ns);
}


Moe::Dataspace *
Moe::Boot_fs::open_file(cxx::String const &name)
{
  dbg.printf("open file '%.*s' from root name space\n", name.len(), name.start());
  Entry *n = root_name_space()->find_iter(name);
  if (n)
    return dynamic_cast<Moe::Dataspace *>(n->obj());

  return 0;
}

