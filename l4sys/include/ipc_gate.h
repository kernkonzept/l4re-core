/**
 * \file
 * The C IPC gate interface.
 *
 * IPC gates are used to create secure communication channels between threads.
 * An IPC gate object can be created using the \ref l4_factory_api interface.
 * With l4_ipc_gate_bind_thread() a thread is bound to an IPC gate which then
 * receives all messages sent to that IPC gate.
 *
 * The l4_ipc_gate_bind_thread() call allows to assign each IPC gate a kernel
 * protected, machine-word sized payload called a *label*. It securely
 * identifies the gate. The lower two bits of the *label* can be used to encode
 * rights bits. The kernel combines these bits with the capability rights, so a
 * programmer usually should not pick the lower two bits for the *label*. The
 * *label* is only visible in the task which is running the thread the IPC gate
 * was bound to and cannot be altered by the sender.
 */
/*
 * (c) 2009-2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
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
#include <l4/sys/rcv_endpoint.h>

/**
 * Bind the IPC gate to a thread.
 * \ingroup l4_kernel_object_gate_api
 *
 * \param gate    The IPC gate object.
 * \param thread  The thread object that shall be bound to `gate`.
 * \param label   Label to assign to `gate`. The two least significant bits
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
l4_ipc_gate_bind_thread(l4_cap_idx_t gate, l4_cap_idx_t thread,
                        l4_umword_t label)
  L4_DEPRECATED("Use l4_rcv_ep_bind_thread().");

/**
 * \internal
 * \ingroup l4_kernel_object_gate_api
 */
L4_INLINE l4_msgtag_t
l4_ipc_gate_bind_thread_u(l4_cap_idx_t gate, l4_cap_idx_t thread,
                          l4_umword_t label, l4_utcb_t *utcb)
  L4_DEPRECATED("Use l4_rcv_ep_bind_thread_u().");

/**
 * \ingroup l4_kernel_object_gate_api
 * \copybrief L4::Ipc_gate::get_infos
 * \param gate  The IPC gate object to get information about.
 * \copydetails L4::Ipc_gate::get_infos
 */
L4_INLINE l4_msgtag_t
l4_ipc_gate_get_infos(l4_cap_idx_t gate, l4_umword_t *label);

/**
 * \internal
 * \ingroup l4_kernel_object_gate_api
 */
L4_INLINE l4_msgtag_t
l4_ipc_gate_get_infos_u(l4_cap_idx_t gate, l4_umword_t *label, l4_utcb_t *utcb);

/**
 * Operations on the IPC-gate.
 * \ingroup l4_protocol_ops
 * \hideinitializer
 * \internal
 */
enum L4_ipc_gate_ops
{
  L4_IPC_GATE_BIND_OP     = 0x10, /**< Bind operation */
  L4_IPC_GATE_GET_INFO_OP = 0x11, /**< Info operation */
};


/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_ipc_gate_bind_thread_u(l4_cap_idx_t gate,
                          l4_cap_idx_t thread, l4_umword_t label,
                          l4_utcb_t *utcb)
{
  return l4_rcv_ep_bind_thread_u(gate, thread, label, utcb);
}

L4_INLINE l4_msgtag_t
l4_ipc_gate_get_infos_u(l4_cap_idx_t gate, l4_umword_t *label, l4_utcb_t *utcb)
{
  l4_msgtag_t tag;
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_IPC_GATE_GET_INFO_OP;
  tag = l4_ipc_call(gate, utcb, l4_msgtag(L4_PROTO_KOBJECT, 1, 0, 0),
                    L4_IPC_NEVER);
  if (!l4_msgtag_has_error(tag) && l4_msgtag_label(tag) >= 0)
    *label = m->mr[0];

  return tag;
}



L4_INLINE l4_msgtag_t
l4_ipc_gate_bind_thread(l4_cap_idx_t gate, l4_cap_idx_t thread,
                        l4_umword_t label)
{
  return l4_rcv_ep_bind_thread_u(gate, thread, label, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_ipc_gate_get_infos(l4_cap_idx_t gate, l4_umword_t *label)
{
  return l4_ipc_gate_get_infos_u(gate, label, l4_utcb());
}
