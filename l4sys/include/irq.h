/**
 * \file
 * C Irq interface
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/kernel_object.h>
#include <l4/sys/ipc.h>
#include <l4/sys/rcv_endpoint.h>

/**
 * \defgroup l4_irq_api IRQs
 * \ingroup  l4_kernel_object_api
 *
 * C IRQ interface, see L4::Irq for the C++ interface.
 *
 * The IRQ interface provides access to abstract interrupts provided by the
 * microkernel. Interrupts may be
 * - hardware interrupts provided by the platform interrupt controller,
 * - virtual device interrupts provided by the microkernel's virtual devices
 *   (virtual serial or trace buffer) or
 * - virtual interrupts that can be triggered by user programs (IRQs) via
 *   l4_irq_trigger().
 *
 * For hardware and virtual device interrupts the Irq object must be bound to
 * an interrupt source, see \ref l4_icu_api. To receive interrupts, the Irq
 * object must be bound to a thread, see l4_rcv_ep_bind_thread() and
 * l4_rcv_ep_bind_snd_destination().
 *
 * IRQ objects can be created using a factory, see the \ref l4_factory_api API
 * (use l4_factory_create_irq()).
 *
 * \includefile{l4/sys/irq.h}
 *
 * For the C++ interface refer to the L4::Irq API for an overview.
 */

/**
 * Detach from an interrupt source.
 * \ingroup l4_irq_api
 *
 * \param irq  The IRQ object that shall be detached.
 *
 * \return Syscall return tag
 *
 * \retval 0           Successfully detached, there was no interrupt pending.
 * \retval 1           Successfully detached, there was an interrupt pending.
 * \retval 2           Successfully detached, an active vIRQ was abandoned.
 * \retval -L4_EPERM   Insufficient permissions; see precondition.
 *
 * \pre The capability `irq` must have the permission #L4_CAP_FPAGE_S.
 */
L4_INLINE l4_msgtag_t
l4_irq_detach(l4_cap_idx_t irq) L4_NOTHROW;

/**
 * \ingroup l4_irq_api
 * \copybrief L4::Irq::detach
 * \param irq  The IRQ object that shall be detached.
 * \copydetails L4::Irq::detach
 */
L4_INLINE l4_msgtag_t
l4_irq_detach_u(l4_cap_idx_t irq, l4_utcb_t *utcb) L4_NOTHROW;


/**
 * \ingroup l4_irq_api
 * \copybrief L4::Irq::bind_vcpu
 *
 * If the interrupt is triggered, the kernel will directly inject the
 * interrupt into the guest. This requires that the thread is currently in
 * extended vCPU user mode. Otherwise the interrupt will stay pending and
 * gets injected on the next vCPU user mode transition. Optionally a doorbell
 * Irq can be registered on the thread (see Thread::register_doorbell_irq())
 * that is triggered in this case.
 *
 * If a guest has acknowledged the interrupt but has not yet issued an EOI
 * (i.e. the interrupt is in "active" state), it is not possible to bind the
 * Irq to a new thread object. Either wait for the guest to issue the EOI or
 * detach() from the current thread. In this case the interrupt will stay
 * active in the guest and it is the responsibility of the VMM to handle the
 * eventual EOI of the guest.
 *
 * \param irq     The IRQ object that shall be bound.
 * \param thread  Thread object this Irq shall be bound to.
 * \param cfg     Architecture specific interrupt configuration.
 *
 * \return Syscall return tag
 *
 * \retval -L4_EPERM   Insufficient permissions; see precondition.
 * \retval -L4_EBUSY   Cannot bind to new thread because interrupt is active
 *                     on previous thread and guest has to issue
 *                     end-of-interrupt first.
 * \retval -L4_ENOSYS  The kernel does not support direct interrupt
 *                     forwarding.
 *
 * \pre The capabilities `irq` and `thread` both must have the permission
 *      #L4_CAP_FPAGE_S.
 */
L4_INLINE l4_msgtag_t
l4_irq_bind_vcpu(l4_cap_idx_t irq, l4_cap_idx_t thread,
                 l4_umword_t cfg) L4_NOTHROW;

/**
 * \ingroup l4_irq_api
 * \copybrief L4::Irq::bind_vcpu
 * \param irq  The IRQ object that shall be bound.
 * \copydetails L4::Irq::bind_vcpu
 */
L4_INLINE l4_msgtag_t
l4_irq_bind_vcpu_u(l4_cap_idx_t irq, l4_cap_idx_t thread, l4_umword_t cfg,
                   l4_utcb_t *utcb) L4_NOTHROW;


/**
 * Trigger an IRQ.
 * \ingroup l4_irq_api
 *
 * \param irq  The IRQ object that shall be triggered.
 *
 * \return Syscall return tag.
 *
 * Note that this function is a send only operation, i.e. there is no return
 * value except for a failed send operation. Especially l4_error() will
 * return an error value from the message tag which still contains the IRQ
 * protocol used for the send operation.
 *
 * Use l4_ipc_error() to check for (send) errors.
 */
L4_INLINE l4_msgtag_t
l4_irq_trigger(l4_cap_idx_t irq) L4_NOTHROW;

/**
 * \ingroup l4_irq_api
 * \copybrief L4::Irq::trigger
 * \param irq  The IRQ object that shall be triggered.
 * \copydetails L4::Irq::trigger
 */
L4_INLINE l4_msgtag_t
l4_irq_trigger_u(l4_cap_idx_t irq, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Unmask and wait for specified IRQ.
 * \ingroup l4_irq_api
 *
 * \param irq  The IRQ object that shall be unmasked.
 * \param to   Timeout.
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_irq_receive(l4_cap_idx_t irq, l4_timeout_t to) L4_NOTHROW;

/**
 * \ingroup l4_irq_api
 * \copybrief L4::Irq::receive
 * \param irq  The IRQ object that shall be unmasked.
 * \copydetails L4::Irq::receive
 */
L4_INLINE l4_msgtag_t
l4_irq_receive_u(l4_cap_idx_t irq, l4_timeout_t timeout, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Unmask IRQ and wait for any message.
 * \ingroup l4_irq_api
 *
 * \param irq    The IRQ object that shall be unmasked.
 * \param label  Receive label.
 * \param to     Timeout.
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_irq_wait(l4_cap_idx_t irq, l4_umword_t *label,
            l4_timeout_t to) L4_NOTHROW;

/**
 * \ingroup l4_irq_api
 * \copybrief L4::Irq::wait
 * \param irq  The IRQ object that shall be unmasked.
 * \copydetails L4::Irq::wait
 */
L4_INLINE l4_msgtag_t
l4_irq_wait_u(l4_cap_idx_t irq, l4_umword_t *label,
              l4_timeout_t timeout, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Unmask IRQ.
 * \ingroup l4_irq_api
 *
 * \param irq  The IRQ object that shall be unmasked.
 *
 * \return Syscall return tag
 *
 * \note l4_irq_wait() and l4_irq_receive() are doing the unmask themselves.
 */
L4_INLINE l4_msgtag_t
l4_irq_unmask(l4_cap_idx_t irq) L4_NOTHROW;

/**
 * \ingroup l4_irq_api
 * \copybrief L4::Irq::unmask
 * \param irq  The IRQ object that shall be unmasked.
 * \copydetails L4::Irq::unmask
 */
L4_INLINE l4_msgtag_t
l4_irq_unmask_u(l4_cap_idx_t irq, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \internal
 */
enum L4_irq_sender_op
{
  L4_IRQ_SENDER_OP_RESERVED1 = 0, // Ex ATTACH
  L4_IRQ_SENDER_OP_DETACH    = 1,
  L4_IRQ_SENDER_OP_BIND_VCPU = 2,
};

/**
 * \internal
 */
enum L4_irq_op
{
  L4_IRQ_OP_TRIGGER   = 2,
  L4_IRQ_OP_EOI       = 4
};

/**************************************************************************
 * Implementations
 */

L4_INLINE l4_msgtag_t
l4_irq_detach_u(l4_cap_idx_t irq, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_IRQ_SENDER_OP_DETACH;
  return l4_ipc_call(irq, utcb, l4_msgtag(L4_PROTO_IRQ_SENDER, 1, 0, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_irq_bind_vcpu_u(l4_cap_idx_t irq, l4_cap_idx_t thread, l4_umword_t cfg,
                   l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_IRQ_SENDER_OP_BIND_VCPU;
  m->mr[1] = cfg;
  m->mr[2] = l4_map_obj_control(0, 0);
  m->mr[3] = l4_obj_fpage(thread, 0, L4_CAP_FPAGE_RWS).raw;
  return l4_ipc_call(irq, utcb, l4_msgtag(L4_PROTO_IRQ_SENDER, 2, 1, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_irq_trigger_u(l4_cap_idx_t irq, l4_utcb_t *utcb) L4_NOTHROW
{
  return l4_ipc_send(irq, utcb, l4_msgtag(L4_PROTO_IRQ, 0, 0, 0),
                     L4_IPC_BOTH_TIMEOUT_0);
}

L4_INLINE l4_msgtag_t
l4_irq_receive_u(l4_cap_idx_t irq, l4_timeout_t to, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_IRQ_OP_EOI;
  return l4_ipc_call(irq, utcb, l4_msgtag(L4_PROTO_IRQ, 1, 0, 0), to);
}

L4_INLINE l4_msgtag_t
l4_irq_wait_u(l4_cap_idx_t irq, l4_umword_t *label,
            l4_timeout_t to, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_IRQ_OP_EOI;
  return l4_ipc_send_and_wait(irq, utcb, l4_msgtag(L4_PROTO_IRQ, 1, 0, 0),
                              label, to);
}

L4_INLINE l4_msgtag_t
l4_irq_unmask_u(l4_cap_idx_t irq, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_IRQ_OP_EOI;
  return l4_ipc_send(irq, utcb, l4_msgtag(L4_PROTO_IRQ, 1, 0, 0), L4_IPC_NEVER);
}


L4_INLINE l4_msgtag_t
l4_irq_detach(l4_cap_idx_t irq) L4_NOTHROW
{
  return l4_irq_detach_u(irq, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_irq_bind_vcpu(l4_cap_idx_t irq, l4_cap_idx_t thread,
                 l4_umword_t cfg) L4_NOTHROW
{
  return l4_irq_bind_vcpu_u(irq, thread, cfg, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_irq_trigger(l4_cap_idx_t irq) L4_NOTHROW
{
  return l4_irq_trigger_u(irq, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_irq_receive(l4_cap_idx_t irq, l4_timeout_t to) L4_NOTHROW
{
  return l4_irq_receive_u(irq, to, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_irq_wait(l4_cap_idx_t irq, l4_umword_t *label,
            l4_timeout_t to) L4_NOTHROW
{
  return l4_irq_wait_u(irq, label, to, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_irq_unmask(l4_cap_idx_t irq) L4_NOTHROW
{
  return l4_irq_unmask_u(irq, l4_utcb());
}

