/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/video/goos>
#include <l4/sys/cxx/ipc_client>

L4_RPC_DEF(L4Re::Video::Goos::get_static_buffer);
L4_RPC_DEF(L4Re::Video::Goos::create_buffer);

