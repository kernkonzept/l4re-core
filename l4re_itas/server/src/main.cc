/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/bid_config.h>
#include <l4/re/namespace>
#include <l4/re/dataspace>
#include <l4/re/mem_alloc>
#include <l4/re/rm>
#include <l4/re/log>
#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/re/util/env_ns>

#include <l4/cxx/exceptions>
#include <l4/cxx/iostream>
#include <l4/cxx/l4iostream>
#include <l4/sys/debugger.h>
#include <l4/sys/scheduler>
#include <l4/sys/thread>
#include <l4/sys/cxx/ipc_server_loop>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "region.h"
#include "globals.h"
#include "loader_elf.h"
#include "debug.h"
#include "dispatcher.h"

#include <l4/re/elf_aux.h>
#include <terminate_handler-l4>

/*
 * Move loader stack out of the way of the default stack address. On no-MMU
 * systems this is not required because there is no address remapping and the
 * stack address is tied to the allocated buffer physical address.
 */
#ifdef CONFIG_MMU
L4RE_ELF_AUX_ELEM_T(l4re_elf_aux_mword_t, __stack_addr,
                    L4RE_ELF_AUX_T_STACK_ADDR,
                    L4_LOADER_RELOC_BASE + 0x1000000);
#endif

using L4::Cap;
using L4Re::Dataspace;

int main(int argc, char const *argv[], char const *env[]);

static Elf_loader loader;

extern "C" void *__libc_alloc_initial_tls(unsigned long size);

void *__libc_alloc_initial_tls(unsigned long)
{
  // __libc_alloc_initial_tls must not be called here, this is just
  // a safety measure
  assert(0);
  return nullptr;
}

static void insert_regions()
{
  using L4Re::Rm;
  using L4Re::Env;

  // Upper limit of a static buffer for storing the returned regions and
  // areas without doing dynamic memory allocation
  enum
  {
    Max_num_regions = L4_UTCB_GENERIC_DATA_SIZE * sizeof(l4_umword_t)
                      / sizeof(Rm::Region),
    Max_num_areas   = L4_UTCB_GENERIC_DATA_SIZE * sizeof(l4_umword_t)
                      / sizeof(Rm::Area),
  };
  union
  {
    Rm::Region r[Max_num_regions];
    Rm::Area a[Max_num_areas];
  } regions_areas;

  int n;
  l4_addr_t addr = 0;
  Rm::Region const *rl;
  while ((n = L4Re::Env::env()->rm()->get_regions(addr, &rl)) > 0)
    {
      assert(sizeof(regions_areas) >= n * sizeof(Rm::Region));
      assert(n <= Max_num_regions);
      // Copy out of UTCB
      memcpy(regions_areas.r, rl, n * sizeof(Rm::Region));

      for (int i = 0; i < n; ++i)
        {
          Rm::Region const *r = &regions_areas.r[i];
          auto pager = L4::cap_reinterpret_cast<L4Re::Dataspace>
            (Env::env()->rm());

          L4::Ipc::String<char> name(0, nullptr);
#ifdef CONFIG_L4RE_REGION_INFO
          char name_buf[50];
          L4Re::Rm::Offset backing_offset;
          name.length = sizeof(name_buf);
          name.data   = name_buf;
          if (L4Re::Env::env()->rm()->get_info(r->start, name, backing_offset))
            name.length = 0;
          else if (name.length > 0) // L4::Ipc::String carries a terminating '\0'
            --name.length;
#endif

          Rm::F::Flags flags = r->flags;
          if (!(flags & (Rm::F::Reserved | Rm::F::Kernel)))
            flags |= Rm::F::Pager;

          void *x = Global::local_rm
            ->attach(reinterpret_cast<void*>(r->start), r->end - r->start + 1,
                     Region_handler(pager, L4_INVALID_CAP, 0,
                                    flags.region_flags()),
                     flags.attach_flags(), L4_PAGESHIFT, name.data, name.length);
          if (x == L4_INVALID_PTR)
            {
              L4::cerr << "l4re: error while initializing RM regions\n";
              exit(1);
            }

          addr = r->end + 1;
        }
    }

  addr = 0;
  Rm::Area const *al;
  while ((n = L4Re::Env::env()->rm()->get_areas(addr, &al)) > 0)
    {
      assert(sizeof(regions_areas) >= n * sizeof(Rm::Area));
      assert(n <= Max_num_areas);
      // Copy out of UTCB
      memcpy(regions_areas.a, al, n * sizeof(Rm::Area));

      for (int i = 0; i < n; ++i)
        {
          Rm::Area const *r = &regions_areas.a[i];
          l4_addr_t x
            = Global::local_rm->attach_area(r->start, r->end - r->start + 1);
          if (x == L4_INVALID_ADDR)
            {
              L4::cerr << "l4re: error while initializing RM areas\n";
              exit(1);
            }

          addr = r->end + 1;
        }
    }
}

static char const *base_name(char const *s)
{
  char *b = strrchr(s, '/');
  return b ? b + 1 : s;
}

int main(int argc, char const *argv[], char const *envp[])
{
  Dbg::set_level(Dbg::Info | Dbg::Warn);

  Dbg boot(Dbg::Boot);

  Global::argc = argc;
  Global::argv = argv;

  if (0)
    {
      L4::cout << "ARGC=" << argc << "\n"
               << "ARGV=" << argv << "\n"
               << "ENVP=" << envp << "\n";

      for (int i = 0; i < argc; ++i)
        L4::cout << "  arg: '" << argv[i] << "'\n";

      for (char const *const *e = Global::envp; *e; ++e)
        L4::cout << "  env: '" << *e << "'\n";
    }

  L4Re::Env *const env = const_cast<L4Re::Env*>(L4Re::Env::env());

  if (0)
      L4::cout << "AUX=" << Global::l4re_aux << "\n"
               << "ENV=" << env << "\n"
               << "Binary: " << Global::l4re_aux->binary << " " << env << "\n";

  if (!env || !Global::l4re_aux)
    {
      Err(Err::Fatal).printf("invalid AUX vectors...\n");
      exit(1);
    }

    {
      char s[15];
      char *t = strstr(Global::l4re_aux->binary, "rom/");
      s[0] = '#';
      strncpy(s + 1, t ? t + 4 : Global::l4re_aux->binary, sizeof(s) - 2);
      s[sizeof(s) - 1] = 0;
      l4_debugger_set_object_name(L4_BASE_THREAD_CAP, s);
      l4_debugger_set_object_name(L4_BASE_TASK_CAP, s + 1);
    }

  Dbg::set_level(Global::l4re_aux->dbg_lvl);
  boot.printf("adding regions from remote region mapper\n");
  insert_regions();

  L4::Cap<Dataspace> file;

  boot.printf("load binary '%s'\n", Global::l4re_aux->binary);

  file = L4Re_app_model::open_file(Global::l4re_aux->binary);

  l4_debugger_add_image_info(L4_BASE_TASK_CAP, Global::l4re_aux->ldr_base,
                             base_name(Global::l4re_aux->binary));
  loader.start(file, Global::local_rm, Global::l4re_aux);

  // Raise RM prio to its MCP
  env->scheduler()->run_thread(env->main_thread(),
                               l4_sched_param(L4_SCHED_MAX_PRIO));

  boot.printf("Start server loop\n");
  server.loop<L4::Runtime_error, Dispatcher &>(dispatcher);
}
