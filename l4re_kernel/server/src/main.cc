/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
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

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "region.h"
#include "globals.h"
#include "loader_elf.h"
#include "debug.h"
#include "dispatcher.h"

#include <l4/re/elf_aux.h>

extern char const __executable_start[];
L4RE_ELF_AUX_ELEM_T(l4re_elf_aux_mword_t, __stack_addr,
                    L4RE_ELF_AUX_T_STACK_ADDR,
                    (l4_addr_t)__executable_start + 0x1000000);

using L4::Cap;
using L4Re::Dataspace;

extern "C" int main(int argc, char const *argv[],
                    char const *env[]);

static Elf_loader loader;
L4::Cap<void> rcv_cap;

class Loop_hooks :
  public L4::Ipc_svr::Ignore_errors,
  public L4::Ipc_svr::Default_timeout,
  public L4::Ipc_svr::Compound_reply
{
public:
  static void setup_wait(l4_utcb_t *utcb, bool)
  {
    l4_utcb_br_u(utcb)->br[0] = L4::Ipc::Small_buf(rcv_cap.cap(),
                                                   L4_RCV_ITEM_LOCAL_ID).raw();
    l4_utcb_br_u(utcb)->br[1] = 0;
    l4_utcb_br_u(utcb)->bdr = 0;
  }
};

extern "C" void *__libc_alloc_initial_tls(unsigned long size);

void *__libc_alloc_initial_tls(unsigned long)
{
  // __libc_alloc_initial_tls must not be called here, this is just
  // a safety measure
  assert(0);
}

static L4::Server<Loop_hooks> server(l4_utcb());

static void insert_regions()
{
  using L4Re::Rm;
  using L4Re::Env;
  int n;
  l4_addr_t addr = 0;
  Rm::Region const *rl;
  while ((n = L4Re::Env::env()->rm()->get_regions(addr, &rl)) > 0)
    {
      for (int i = 0; i < n; ++i)
        {
          Rm::Region const *r = &rl[i];
          auto pager = L4::cap_reinterpret_cast<L4Re::Dataspace>(Env::env()->rm());
          void *x = Global::local_rm
            ->attach((void*)r->start, r->end - r->start +1,
                     Region_handler(pager, L4_INVALID_CAP, 0, Rm::Pager), 0);
          if (!x)
            {
              L4::cerr << "l4re: error while initializing region mapper\n";
              exit(1);
            }

          addr = r->end + 1;
        }

    }

  Rm::Area const *al;
  while ((n = L4Re::Env::env()->rm()->get_areas(addr, &al)) > 0)
    {
      for (int i = 0; i < n; ++i)
        {
          Rm::Area const *r = &al[i];
          l4_addr_t x
            = Global::local_rm->attach_area(r->start, r->end - r->start + 1);
          if (!x)
            {
              L4::cerr << "l4re: error while initializing region mapper\n";
              exit(1);
            }

          addr = r->end + 1;
        }
    }
}

static int run(int argc, char const *argv[], char const *envp[])
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
      strncpy(s + 1, t ? t + 4 : Global::l4re_aux->binary, sizeof(s)-1);
      s[sizeof(s)-1] = 0;
      l4_debugger_set_object_name(L4_BASE_THREAD_CAP, s);
      l4_debugger_set_object_name(L4_BASE_TASK_CAP, s + 1);
    }

  Dbg::set_level(Global::l4re_aux->dbg_lvl);
  rcv_cap = Global::cap_alloc->alloc<void>();
  boot.printf("adding regions from remote region mapper\n");
  insert_regions();

  L4::Cap<Dataspace> file;

  boot.printf("load binary '%s'\n", Global::l4re_aux->binary);

  file = L4Re_app_model::open_file(Global::l4re_aux->binary);

  loader.start(file, Global::local_rm, Global::l4re_aux);

  // Raise RM prio to its MCP
  env->scheduler()->run_thread(env->main_thread(),
                               l4_sched_param(L4_SCHED_MAX_PRIO));

  boot.printf("Start server loop\n");
  server.loop<L4::Runtime_error>(Dispatcher());
}

int main(int argc, char const *argv[], char const *envp[])
{
  try
    {
      return run(argc, argv, envp);
    }
  catch (L4::Runtime_error const &e)
    {
      Err(Err::Fatal).printf("Exception %s: '%s'\n", e.str(), e.extra_str());
      L4::cerr << e;
      return 1;
    }
  catch (L4::Base_exception const &e)
    {
      Err(Err::Fatal).printf("Exception %s\n", e.str());
      L4::cerr << e;
      return 1;
    }

  return 0;
}
