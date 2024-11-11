/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/event>
#include <l4/sys/cxx/ipc_client>

L4_RPC_DEF(L4Re::Event::get_buffer);
L4_RPC_DEF(L4Re::Event::get_num_streams);
L4_RPC_DEF(L4Re::Event::get_stream_info);
L4_RPC_DEF(L4Re::Event::get_stream_info_for_id);
L4_RPC_DEF(L4Re::Event::get_axis_info);
L4_RPC_DEF(L4Re::Event::get_stream_state_for_id);



