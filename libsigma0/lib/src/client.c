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

/**
 * New client.
 *
 * \param pager  pager implementing Sigma0 protocol
 * \param gate   slot to attach to
 * \return           #0                 on success
 *                  -#L4SIGMA0_IPCERROR IPC error
 *                  -#L4SIGMA0_NOFPAGE  no fpage received
 */
L4_CV int
l4sigma0_new_client(l4_cap_idx_t pager, l4_cap_idx_t gate)
{
  l4_msgtag_t tag = l4_msgtag(L4_PROTO_SIGMA0, 1, 0, 0);
  l4_utcb_t *utcb = l4_utcb();
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  l4_buf_regs_t *b = l4_utcb_br_u(utcb);

  m->mr[0] = SIGMA0_REQ_NEW_CLIENT;
  b->bdr = 0;
  b->br[0] = L4_ITEM_MAP;
  b->br[1] = l4_obj_fpage(gate, 0, 0).raw;

  tag = l4_ipc_call(pager, utcb, tag, L4_IPC_NEVER);
  if (l4_msgtag_has_error(tag))
    return -L4SIGMA0_IPCERROR;

  if (l4_msgtag_items(tag) != 1)
    return -L4SIGMA0_NOFPAGE;

  return 0;
}
