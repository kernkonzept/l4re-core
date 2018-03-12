/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "app_model.h"
#include "debug.h"

#include <l4/libloader/elf>
#include <l4/cxx/iostream>
#include <l4/util/util.h>
#include <cstdio>

#include "lua.h"
#include "server.h"

static Dbg info(Dbg::Info);
static Dbg boot_info(Dbg::Boot);
l4re_aux_t* l4re_aux;

//static Ned::Server s(l4_utcb(), Ned::Registry(L4Re::Env::env()->main_thread(), L4Re::Env::env()->factory()));


Ned::Server *Ned::server;// = &s;

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

  l4_umword_t *auxp = (l4_umword_t*)&argv[argc] + 1;
  while (*auxp)
    ++auxp;
  ++auxp;

  l4re_aux = 0;

  while (*auxp)
    {
      if (*auxp == 0xf0)
	l4re_aux = (l4re_aux_t*)auxp[1];
      auxp += 2;
    }

  Ned::Server svr;
  Ned::server = &svr;


  lua(argc, argv);


  while (1)
    l4_sleep_forever();

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

  l4_sleep_forever();
  return 0;
}
