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

namespace Ned {

extern L4Re::Util::Registry_server<L4Re::Util::Br_manager_timeout_hooks> server;

void server_loop();

}
