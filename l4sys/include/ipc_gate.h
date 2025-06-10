/*
 * (c) 2009-2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * The C IPC gate interface, see L4::Ipc_gate for the C++ interface.
 *
 * IPC gates are used to create secure communication channels between protection
 * domains. An IPC gate can be created using the \ref l4_factory_api interface.
 *
 * Depending on the permissions of the capability used, an IPC gate forwards IPC
 * to the \ref l4_thread_api the IPC gate is *bound* to (cf.
 * l4_rcv_ep_bind_thread() and l4_rcv_ep_bind_snd_destination()). If the
 * capability has the #L4_FPAGE_C_IPCGATE_SVR permission, only IPC using a
 * protocol different from the #L4_PROTO_KOBJECT protocol is forwarded. Without
 * the #L4_FPAGE_C_IPCGATE_SVR permission, all IPC is forwarded. The latter is
 * the usual case for a client in a client/server scenario. When not bound to a
 * thread or thread group yet, the forwarded IPC blocks until the IPC gate is
 * bound to a thread or thread group, or the IPC times out.
 *
 * Forwarded IPC is always forwarded to the userland of the thread the IPC gate
 * is bound to, either directly or indirectly using a thread group. That means,
 * the \ref l4_thread_api interface of that thread is not accessible via an IPC
 * gate. The \ref l4_kernel_object_gate_api of an IPC gate is only accessible if
 * the capability used has the #L4_FPAGE_C_IPCGATE_SVR permission (cf. previous
 * paragraph). Conversely that means, if the capability used lacks the
 * #L4_FPAGE_C_IPCGATE_SVR permission, \ref l4_kernel_object_gate_api calls are
 * forwarded to the thread or thread group the IPC gate is bound to instead of
 * being processed by the IPC gate itself. In a client/server scenario, a client
 * should only get IPC gate capabilities without #L4_FPAGE_C_IPCGATE_SVR
 * permission so the client cannot tamper with the IPC gate.
 *
 * When binding an IPC gate to a thread or thread group, a user-defined, kernel
 * protected, machine-word sized payload called the IPC gate’s *label* is
 * assigned to the IPC gate (note that the two least significant bits of the
 * label must be zero; cf. l4_rcv_ep_bind_thread() and
 * l4_rcv_ep_bind_snd_destination()). When a send-only IPC or call IPC is
 * forwarded via an IPC gate, the label provided by the sender is ignored and
 * replaced by the IPC gate’s label where the two least significant bits are set
 * to the #L4_CAP_FPAGE_S and #L4_CAP_FPAGE_W permissions of the capability
 * used. The replaced label is only visible to the thread the IPC gate is bound
 * to upon receive (or to the selected thread from the thread group the IPC gate
 * is bound to). However, the configured label of an IPC gate can also be
 * queried via l4_ipc_gate_get_infos() if the capability used has the
 * #L4_FPAGE_C_IPCGATE_SVR permission.
 *
 * When deleting an IPC gate or when unbinding it from a thread or thread group,
 * the label of IPC already in flight won't be changed. To ensure that no IPC
 * from this IPC gate is received by a thread with an unexpected label,
 * l4_thread_modify_sender_start() shall be used to change the labels of every
 * pending IPC to that gate. This is also required if the label of an already
 * bound IPC gate is changed. It is not necessary after binding the IPC gate to
 * a thread or thread group for the first time.
 *
 * When binding a currently bound IPC gate to a new thread or thread group, the
 * same label should be used that was used with the old thread. Otherwise the
 * old and the new thread need to synchronize to avoid IPC messages with
 * unexpected labels.
 *
 * \includefile{l4/sys/ipc_gate.h}
 *
 * For the C++ interface refer to the L4::Ipc_gate documentation.
 *
 * \see \ref l4_ipc_api
 */

#pragma once

#include <l4/sys/utcb.h>
#include <l4/sys/types.h>
#include <l4/sys/rcv_endpoint.h>

/**
 * \ingroup l4_kernel_object_gate_api
 * \copybrief L4::Ipc_gate::get_infos
 *
 * \param gate  The IPC gate object to get information about.
 * \param[out] label  The label of the IPC gate is returned here.
 *
 * \return System call return tag.
 *
 * \pre If `gate` does not possess the #L4_FPAGE_C_IPCGATE_SVR right, the kernel
 *      will not perform this operation. Instead, the underlying IPC message
 *      will be forwarded to the thread or thread group the IPC gate is bound
 *      to, blocking the caller if the IPC gate is not bound yet.
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
l4_ipc_gate_bind_snd_destination_u(l4_cap_idx_t gate,
                                   l4_cap_idx_t snd_dst, l4_umword_t label,
                                   l4_utcb_t *utcb)
{
  return l4_rcv_ep_bind_snd_destination_u(gate, snd_dst, label, utcb);
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
l4_ipc_gate_bind_snd_destination(l4_cap_idx_t gate, l4_cap_idx_t snd_dst,
                                 l4_umword_t label)
{
  return l4_rcv_ep_bind_snd_destination_u(gate, snd_dst, label, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_ipc_gate_get_infos(l4_cap_idx_t gate, l4_umword_t *label)
{
  return l4_ipc_gate_get_infos_u(gate, label, l4_utcb());
}
