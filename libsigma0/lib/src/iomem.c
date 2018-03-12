/**
 * \file	sigma0/lib/src/iomem.c
 * \brief	map memory-mapped I/O memory using sigma0 protocol
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


L4_CV int
l4sigma0_map_iomem(l4_cap_idx_t pager,
                   l4_addr_t phys, l4_addr_t virt, l4_addr_t size, int cached)
{
  l4_addr_t    d = L4_SUPERPAGESIZE;
  unsigned     l = L4_LOG2_SUPERPAGESIZE;
  l4_msgtag_t  tag;
  l4_utcb_t *utcb = l4_utcb();
  int error;

  if ((phys & (d-1)) || (size & (d-1)) || (virt & (d-1)))
    {
      l = L4_LOG2_PAGESIZE;
      d = L4_PAGESIZE;
    }

  if ((phys & (d-1)) || (size & (d-1)) || (virt & (d-1)))
    return -L4SIGMA0_NOTALIGNED;

  for (; size>0; phys+=d, size-=d, virt+=d)
    {
      do
	{
	  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
	  l4_buf_regs_t *b = l4_utcb_br_u(utcb);
	  tag = l4_msgtag(L4_PROTO_SIGMA0, 2, 0, 0);
	  m->mr[0] = cached ? SIGMA0_REQ_FPAGE_IOMEM_CACHED
				   : SIGMA0_REQ_FPAGE_IOMEM;
	  m->mr[1] = l4_fpage(phys, l, L4_FPAGE_RWX).raw;

	  b->bdr   = 0;
	  b->br[0] = L4_ITEM_MAP;
	  b->br[1] = l4_fpage(virt, l, L4_FPAGE_RWX).raw;
	  tag = l4_ipc_call(pager, utcb, tag, L4_IPC_NEVER);
	  if (l4_msgtag_has_error(tag))
	    error = l4_utcb_tcr_u(utcb)->error;
	  else
	    error = 0;
	}
      while (error == L4_IPC_SECANCELED || error == L4_IPC_SEABORTED);

      if (error)
	return -L4SIGMA0_IPCERROR;

      if (l4_msgtag_items(tag) < 1)
	return -L4SIGMA0_NOFPAGE;
    }

  return 0;
}
