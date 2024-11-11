/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/cxx/list>
#include <l4/re/error_helper>

#include "server.h"

namespace Ned {

L4Re::Util::Registry_server<Termination_loop_hooks> server;

bool Termination_loop_hooks::wait_for_apps = false;

void server_loop(bool wait_for_apps)
{
  static bool once;

  if (once)
    return;
  once = true;

  Termination_loop_hooks::wait_for_apps = wait_for_apps;
  server.loop();
}

}
