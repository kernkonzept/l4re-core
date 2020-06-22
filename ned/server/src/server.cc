/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/cxx/list>
#include <l4/re/error_helper>

#include "server.h"

namespace Ned {

L4Re::Util::Registry_server<L4Re::Util::Br_manager_timeout_hooks> server;

void server_loop()
{
  static bool once;

  if (once)
    return;
  once = true;

  server.loop();
}

}
