/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/ipc.h>
#include <l4/sigma0/sigma0.h>

L4_CV void
l4sigma0_debug_dump(l4_cap_idx_t pager)
{
  l4_msgtag_t tag = l4_msgtag(L4_PROTO_SIGMA0, 1, 0, 0);
  l4_utcb_mr()->mr[0] = SIGMA0_REQ_DEBUG_DUMP;

  l4_ipc_call(pager, l4_utcb(), tag, L4_IPC_NEVER);
}
