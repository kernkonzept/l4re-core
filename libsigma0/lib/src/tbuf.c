/**
 * \file	sigma0/lib/src/tbuf.c
 * \brief	map tracebuffer descriptor using sigma0 protocol
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
 * Map the Fiasco tracebuffer status descriptor using the Sigma0 protocol.
 *
 * \param pager  pager implementing the Sigma0 protocol
 * \param virt   virtual address the descriptor should be mapped to
 * \return  #0                 on success
 *         -#L4SIGMA0_IPCERROR IPC error
 *         -#L4SIGMA0_NOFPAGE  no fpage received
 */
L4_CV int
l4sigma0_map_tbuf(l4_cap_idx_t pager, l4_addr_t virt)
{
  //assert(0);
  // hand-made assert:
  __builtin_trap();
  (void)pager;
  (void)virt;
#if 0
  l4_msgdope_t result;
  int error;
  l4_msgtag_t tag = l4_msgtag(L4_MSGTAG_SIGMA0, 0, 0, 0);

  error = l4_ipc_call(pager, tag, L4_IPC_NEVER, &result, &tag);

  if (error)
    return -L4SIGMA0_IPCERROR;

  if (fpage.fpage == 0 || !l4_ipc_fpage_received(result))
    return -L4SIGMA0_NOFPAGE;
#endif
  return 0;
}
