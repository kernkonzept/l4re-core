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

#include <l4/sys/kernel_object.h>
#include <l4/sys/ipc.h>

/**
 * \defgroup l4_irq_api IRQs
 * \ingroup  l4_kernel_object_api
 *
 * C IRQ interface.
 *
 * The IRQ interface provides access to abstract interrupts provided by the
 * microkernel. Interrupts may be
 * - hardware interrupts provided by the platform interrupt controller,
 * - virtual device interrupts provided by the microkernel's virtual devices
 *   (virtual serial or trace buffer) or
 * - virtual interrupts that can be triggered by user programs (IRQs)
 *
 * IRQ objects can be created using a factory, see the \ref l4_factory_api API
 * (use l4_factory_create_irq()).
 *
 * \includefile{l4/sys/irq.h}
 *
 * For the C++ interface refer to the L4::Irq API for an overview.
 */

/**
 * Attach a thread to an interrupt source.
 * \ingroup l4_irq_api
 *
 * \param irq     IRQ object where `thread` is attached to.
 * \param label   Identifier of the IRQ.
 * \param thread  The thread object to attach `irq` to.
 *
 * \return Syscall return tag
 */
L4_INLINE l4_msgtag_t
l4_irq_attach(l4_cap_idx_t irq, l4_umword_t label,
              l4_cap_idx_t thread) L4_NOTHROW;

/**
 * \ingroup l4_irq_api
 * \copybrief L4::Irq::attach
 * \param irq  IRQ object where `thread` is attached to.
 * \copydetails L4::Irq::attach
 */
L4_INLINE l4_msgtag_t
l4_irq_attach_u(l4_cap_idx_t irq, l4_umword_t label,
                l4_cap_idx_t thread, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Chain an IRQ to another master IRQ source.
 * \ingroup l4_irq_api
 *
 * \param irq    The master IRQ object.
 * \param slave  The slave that shall be attached to the master.
 *
 * \return Syscall return tag
 *
 * The chaining feature of IRQ objects allows to deal with shared IRQs. For
 * chaining IRQs there must be a master IRQ object, bound to the real IRQ
 * source. Note, the master IRQ must not have a thread attached to it.
 *
 * This function allows to add a limited number of slave IRQs to this master
 * IRQ, with the semantics that each of the slave IRQs is triggered whenever
 * the master IRQ is triggered. The master IRQ will be masked automatically
 * when an IRQ is delivered and shall be unmasked when all attached slave IRQs
 * are unmasked.
 */
L4_INLINE l4_msgtag_t
l4_irq_mux_chain(l4_cap_idx_t irq, l4_cap_idx_t slave) L4_NOTHROW;

/**
 * \ingroup l4_irq_api
 * \copybrief L4::Irq_mux::chain
 * \param irq  The master IRQ object.
 * \copydetails L4::Irq_mux::chain
 */
L4_INLINE l4_msgtag_t
l4_irq_mux_chain_u(l4_cap_idx_t irq, l4_cap_idx_t slave,
                   l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Detach from an interrupt source.
 * \ingroup l4_irq_api
 *
 * \param irq  The IRQ object that shall be detached.
 *
 * \return Syscall return tag
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
  L4_IRQ_SENDER_OP_ATTACH    = 0,
  L4_IRQ_SENDER_OP_DETACH    = 1
};

/**
 * \internal
 */
enum L4_irq_mux_op
{
  L4_IRQ_MUX_OP_CHAIN = 0
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
l4_irq_attach_u(l4_cap_idx_t irq, l4_umword_t label,
                l4_cap_idx_t thread, l4_utcb_t *utcb) L4_NOTHROW
{
  int items = 0;
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_IRQ_SENDER_OP_ATTACH;
  m->mr[1] = label;

  if (!l4_is_invalid_cap(thread))
    {
      items = 1;
      m->mr[2] = l4_map_obj_control(0, 0);
      m->mr[3] = l4_obj_fpage(thread, 0, L4_FPAGE_RWX).raw;
    }
  return l4_ipc_call(irq, utcb, l4_msgtag(L4_PROTO_IRQ_SENDER, 2, items, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_irq_mux_chain_u(l4_cap_idx_t irq, l4_cap_idx_t slave,
                   l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_IRQ_MUX_OP_CHAIN;
  m->mr[1] = l4_map_obj_control(0, 0);
  m->mr[2] = l4_obj_fpage(slave, 0, L4_FPAGE_RWX).raw;
  return l4_ipc_call(irq, utcb, l4_msgtag(L4_PROTO_IRQ_MUX, 1, 1, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_irq_detach_u(l4_cap_idx_t irq, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_IRQ_SENDER_OP_DETACH;
  return l4_ipc_call(irq, utcb, l4_msgtag(L4_PROTO_IRQ_SENDER, 1, 0, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_irq_trigger_u(l4_cap_idx_t irq, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_utcb_mr_u(utcb)->mr[0] = L4_IRQ_OP_TRIGGER;
  return l4_ipc_send(irq, utcb, l4_msgtag(L4_PROTO_IRQ, 1, 0, 0),
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
l4_irq_attach(l4_cap_idx_t irq, l4_umword_t label,
              l4_cap_idx_t thread) L4_NOTHROW
{
  return l4_irq_attach_u(irq, label, thread, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_irq_mux_chain(l4_cap_idx_t irq, l4_cap_idx_t slave) L4_NOTHROW
{
  return l4_irq_mux_chain_u(irq, slave, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_irq_detach(l4_cap_idx_t irq) L4_NOTHROW
{
  return l4_irq_detach_u(irq, l4_utcb());
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

