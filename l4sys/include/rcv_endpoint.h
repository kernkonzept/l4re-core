/**
 * \file
 * Receive endpoint C interface.
 */
/*
 * (c) 2017 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/utcb.h>
#include <l4/sys/types.h>

/**
 * Bind the IPC receive endpoint to a thread.
 * \ingroup l4_kernel_object_gate_api
 *
 * \param ep      The IPC receive endpoint object.
 * \param thread  The thread object `ep` shall be bound to.
 * \param label   Label to assign to `ep`. For IPC gates, the two least
 *                significant bits must be set to zero.
 *
 * \return Syscall return tag containing one of the following return codes.
 *
 * \retval L4_EOK      Operation successful.
 * \retval -L4_EINVAL  `thread` is not a thread object or other arguments were
 *                     malformed.
 * \retval -L4_EPERM   Insufficient permissions; see precondition.
 *
 * \pre The capabilities `ep` and `thread` both must have the permission
 *      #L4_CAP_FPAGE_S.
 *
 * \pre If `ep` is an IPC gate capability without the #L4_FPAGE_C_IPCGATE_SVR
 *      right, the kernel will not perform this operation. Instead, the
 *      underlying IPC message will be forwarded to the thread the IPC gate is
 *      bound to, blocking the caller if the IPC gate was not bound yet.
 *
 *  The specified `label` is passed to the receiver of the incoming IPC. It is
 *  possible to re-bind a receive endpoint to the same or a different thread.
 *  In this case, IPC already in flight will be delivered with the old label to
 *  the previously bound thread unless l4_thread_modify_sender_start() is used
 *  to change these labels.
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

/// Receive endpoint operations.
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
  m->mr[3] = l4_obj_fpage(thread, 0, L4_CAP_FPAGE_RWS).raw;
  return l4_ipc_call(ep, utcb, l4_msgtag(L4_PROTO_KOBJECT, 2, 1, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_rcv_ep_bind_thread(l4_cap_idx_t ep, l4_cap_idx_t thread,
                      l4_umword_t label)
{
  return l4_rcv_ep_bind_thread_u(ep, thread, label, l4_utcb());
}


