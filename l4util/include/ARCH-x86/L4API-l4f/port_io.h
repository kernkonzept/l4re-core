/*****************************************************************************/
/**
 * \file
 * \brief  port I/O functions
 *
 * \date   06/2003
 * \author Frank Mehnert <fm3@os.inf.tu-dresden.de>
 */
/*****************************************************************************/

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef _L4UTIL_PORT_IO_API_H
#define _L4UTIL_PORT_IO_API_H

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>

#include <x86/l4/util/port_io.h>

EXTERN_C_BEGIN

/**
 * \brief Map a range of I/O ports.
 * \ingroup l4util_port_io
 *
 * \param sigma0id   I/O port service (sigma0).
 * \param port_start (Start) Port to request.
 * \param log2size   Log2size of range to request.
 *
 * \return IPC result: 0 if the range could be successfully mapped
 *         on error: IPC failure, or -L4_ENOENT if nothing mapped
 */
L4_INLINE int
l4util_ioport_map(l4_cap_idx_t sigma0id,
                  unsigned port_start, unsigned log2size);

EXTERN_C_END


/*****************************************************************************
 *** Implementation
 *****************************************************************************/

#include <l4/sys/utcb.h>
#include <l4/sys/ipc.h>


L4_INLINE int
l4util_ioport_map(l4_cap_idx_t sigma0id,
                  unsigned port_start, unsigned log2size)
{
  l4_fpage_t iofp;
  l4_msgtag_t tag;
  long err;

  iofp = l4_iofpage(port_start, log2size);
  l4_utcb_mr()->mr[0] = iofp.raw;
  l4_utcb_br()->bdr   = 0;
  l4_utcb_br()->br[0] = L4_ITEM_MAP;
  l4_utcb_br()->br[1] = iofp.raw;
  tag = l4_ipc_call(sigma0id, l4_utcb(),
	            l4_msgtag(L4_PROTO_IO_PAGE_FAULT, 1, 0, 0),
                    L4_IPC_NEVER);

  if ((err = l4_ipc_error(tag, l4_utcb())))
    return err;

  return l4_msgtag_items(tag) > 0 ? 0 : -L4_ENOENT;
}

#endif

