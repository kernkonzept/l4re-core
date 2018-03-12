/**
 * \file	sigma0/lib/src/iomem.c
 * \brief	map any page using sigma0 protocol
 *
 * \date	02/2006
 * \author	Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2006-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/ipc.h>
#include <l4/sigma0/sigma0.h>


/**
 * Map one page anonymous memory.
 *
 * \param pager          pager implementing the Sigma0 protocol
 * \param map_area       virtual address of the map area
 * \param log2_map_size  size of the map area
 * \param sz             Size to map from the server, in log2.
 * \return           #0                 on success
 *                  -#L4SIGMA0_IPCERROR IPC error
 *                  -#L4SIGMA0_NOFPAGE  no fpage received
 */
L4_CV int
l4sigma0_map_anypage(l4_cap_idx_t pager, l4_addr_t map_area,
		     unsigned log2_map_size, l4_addr_t *base, unsigned sz)
{
  l4_msgtag_t tag = l4_msgtag(L4_PROTO_SIGMA0, 2, 0, 0);
  l4_utcb_t *utcb = l4_utcb();
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  l4_buf_regs_t *b = l4_utcb_br_u(utcb);

  m->mr[0] = SIGMA0_REQ_FPAGE_ANY;
  m->mr[1] = l4_fpage(0, sz, 0).raw;

  b->bdr = 0;
  b->br[0] = L4_ITEM_MAP;
  b->br[1] = l4_fpage(map_area, log2_map_size, L4_FPAGE_RWX).raw;

  tag = l4_ipc_call(pager, utcb, tag, L4_IPC_NEVER);
  if (l4_ipc_error(tag, utcb))
    return -L4SIGMA0_IPCERROR;

  if (l4_msgtag_items(tag) != 1)
    return -L4SIGMA0_NOFPAGE;

  *base = m->mr[0] & (~0UL << L4_PAGESHIFT);

  return 0;
}
