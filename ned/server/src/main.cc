/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "app_model.h"
#include "debug.h"

#include <l4/libloader/elf>
#include <l4/cxx/iostream>
#include <l4/util/util.h>
#include <cstdio>

#include "lua.h"
#include "foreign_server.h"
#include "server.h"

static Dbg info(Dbg::Info);
static Dbg boot_info(Dbg::Boot);
l4re_aux_t const* l4re_aux;

Ned::Foreign_server *Ned::foreign_server;

static Dbg ldr(Dbg::Loader, "ldr");

static
int
run(int argc, char const *const *argv)
{
  Dbg::set_level(Dbg::Warn);
  info.printf("Hello from Ned\n");

  boot_info.printf("cmdline: ");
  for (int i = 0; i < argc; ++i)
    boot_info.cprintf("%s ", argv[i]);
  boot_info.cprintf("\n");

  auto auxp = &argv[argc] + 1;
  while (*auxp)
    ++auxp;
  ++auxp;

  l4re_aux = 0;

  auto *sentinel = reinterpret_cast<char const*>(0xf0);
  while (*auxp)
    {
      if (*auxp == sentinel)
        l4re_aux = reinterpret_cast<l4re_aux_t const*>(auxp[1]);
      auxp += 2;
    }

  static Ned::Foreign_server svr;
  Ned::foreign_server = &svr;

  bool exit_opt = false;
  bool wait_opt = false;
  if (argc > 1)
    {
      exit_opt = !strcmp(argv[1], "--exit");
      wait_opt = !strcmp(argv[1], "--wait-and-exit");
      if (exit_opt || wait_opt)
        {
          argv++;
          argc--;
        }
    }

  lua(argc, argv);

  if (!exit_opt)
    Ned::server_loop(wait_opt);

  return 0;
};

int
main(int argc, char const *const *argv)
{
  try
    {
      return run(argc, argv);
    }
  catch (L4::Runtime_error &e)
    {
      L4::cerr << "FATAL: " << e;
      l4_sleep_forever();
    }
  catch (Ned::App_termination)
    {
      return 0;
    }


  l4_sleep_forever();
  return 0;
}
