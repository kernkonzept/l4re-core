/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/ref_ptr>
#include <l4/cxx/slist>
#include <l4/re/util/br_manager>
#include <l4/re/util/object_registry>

#include "app_task.h"

namespace Ned {

struct App_termination {};

struct Termination_loop_hooks :
  public L4::Ipc_svr::Timeout_queue_hooks<L4Re::Util::Br_manager_timeout_hooks,
                                          L4Re::Util::Br_manager>,
  public L4::Ipc_svr::Ignore_errors
{
  void setup_wait(l4_utcb_t *utcb, L4::Ipc_svr::Reply_mode r)
  {
    if (wait_for_apps && !App_task::has_apps_running())
      throw App_termination();

    Br_manager::setup_wait(utcb, r);
  }

  static bool wait_for_apps;
};

extern L4Re::Util::Registry_server<Termination_loop_hooks> server;

void server_loop(bool wait_for_apps);

}
