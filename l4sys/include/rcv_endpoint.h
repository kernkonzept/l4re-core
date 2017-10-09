/**
 * \file
 * Receive endpoint C interface.
 */
/*
 * (c) 2017 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#include <l4/sys/utcb.h>
#include <l4/sys/types.h>

/**
 * Bind the IPC gate to a thread.
 * \ingroup l4_kernel_object_gate_api
 *
 * \param ep      The IPC receive endpoint object.
 * \param thread  The thread object that shall be bound to `ep`.
 * \param label   Label to assign to `ep`. The two least significant bits
 *                should usually be set to zero.
 *
 * \return Syscall return tag containing one of the following return codes.
 *
 * \retval L4_EOK      Operation successful.
 * \retval -L4_EINVAL  `thread` is not a thread object or other arguments were
 *                     malformed.
 * \retval -L4_EPERM   `thread` is missing #L4_CAP_FPAGE_S right.
 */

L4_INLINE l4_msgtag_t
l4_rcv_ep_bind_thread(l4_cap_idx_t ep, l4_cap_idx_t thread,
                      l4_umword_t label);

/**
 * \internal
 * \ingroup l4_kernel_object_gate_api
 */
L4_INLINE l4_msgtag_t
l4_rcv_ep_bind_thread_u(l4_cap_idx_t ep, l4_cap_idx_t thread,
                        l4_umword_t label, l4_utcb_t *utcb);

enum L4_rcv_ep_ops
{
  L4_RCV_EP_BIND_OP     = 0x10, /**< Bind operation */
};

/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_rcv_ep_bind_thread_u(l4_cap_idx_t ep,
                        l4_cap_idx_t thread, l4_umword_t label,
                        l4_utcb_t *utcb)
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_RCV_EP_BIND_OP;
  m->mr[1] = label;
  m->mr[2] = l4_map_obj_control(0, 0);
  m->mr[3] = l4_obj_fpage(thread, 0, L4_FPAGE_RWX).raw;
  return l4_ipc_call(ep, utcb, l4_msgtag(L4_PROTO_KOBJECT, 2, 1, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_rcv_ep_bind_thread(l4_cap_idx_t ep, l4_cap_idx_t thread,
                      l4_umword_t label)
{
  return l4_rcv_ep_bind_thread_u(ep, thread, label, l4_utcb());
}


