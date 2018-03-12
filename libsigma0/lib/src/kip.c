/**
 * \file	sigma0/lib/src/kip.c
 * \brief	map kernel info page using sigma0 protocol
 *
 * \date	02/2006
 * \author	Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *		Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2006-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/ipc.h>
#include <l4/sigma0/sigma0.h>

/*
 * \return kip address, 0 on error
 */
L4_CV l4_kernel_info_t *
l4sigma0_map_kip(l4_cap_idx_t pager, void *adr, unsigned log2_size)
{
  l4_msgtag_t tag = l4_msgtag(L4_PROTO_SIGMA0, 1, 0, 0);
  l4_utcb_t *utcb = l4_utcb();
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  l4_buf_regs_t *b = l4_utcb_br_u(utcb);

  l4_addr_t addr = (l4_addr_t)adr & (~0ULL << log2_size);
  m->mr[0] = SIGMA0_REQ_KIP;
  b->bdr = 0;
  b->br[0] = L4_ITEM_MAP;
  b->br[1] = l4_fpage(addr, log2_size, L4_FPAGE_RX).raw;

  tag = l4_ipc_call(pager, utcb, tag, L4_IPC_NEVER);
  if (l4_ipc_error(tag, utcb))
    return 0;

  if (l4_msgtag_items(tag) != 1)
    return 0;

  l4_addr_t a = addr + (m->mr[0] & (~0UL << L4_PAGESHIFT) & ((1ULL << log2_size)-1));

  return (l4_kernel_info_t*)a;
}

