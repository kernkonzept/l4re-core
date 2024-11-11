/**
 * \file
 * C semaphore interface
 * \ingroup l4_api
 */
/*
 * (c) 2015 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <l4/sys/irq.h>

/**
 * \defgroup l4_semaphore_api Kernel-provided semaphore
 * \ingroup  l4_kernel_object_api
 *
 * C semaphore interface, see L4::Semaphore for the C++ interface.
 *
 * \includefile{l4/sys/semaphore.h}
 */

enum L4_semaphore_op
{
  L4_SEMAPHORE_OP_DOWN    = 0,
  // semaphore up is IRQ_OP_TRIGGER with IRQ/Triggerable protocol
};

/**
 * \ingroup l4_semaphore_api
 * \copybrief L4::Semaphore::up()
 *
 * \param sem  Semaphore object.
 *
 * \return Send-only IPC message return tag. Use l4_ipc_error() to check for
 *         errors, do **not** use l4_error().
 *
 * Increases the semaphore counter by one if it is smaller than an
 * unspecified limit. The unspecified limit is guaranteed to be at
 * least 2^31-1.
 */
L4_INLINE l4_msgtag_t
l4_semaphore_up(l4_cap_idx_t sem) L4_NOTHROW
{
  return l4_irq_trigger(sem);
}

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_semaphore_up_u(l4_cap_idx_t sem, l4_utcb_t *utcb) L4_NOTHROW
{
  return l4_irq_trigger_u(sem, utcb);
}

/**
 * \ingroup l4_semaphore_api
 * \copybrief L4::Semaphore::down()
 *
 * \param sem  Semaphore object.
 * \param timeout  Timeout for blocking the semaphore down operation.
 *                 Note: The receive timeout of this timeout-pair is
 *                 significant for blocking, the send part is usually
 *                 non-blocking.
 *
 * \return Syscall return tag. Use l4_error() to check for errors.
 * \retval -L4_EPERM  No #L4_CAP_FPAGE_S right on invoked semaphore
 *                    capability.
 *
 * This method decrements the semaphore counter by one, or blocks if the
 * counter is already zero, until either a timeout or cancel condition hits
 * or the counter is increased by an `up()` operation.
 */
L4_INLINE l4_msgtag_t
l4_semaphore_down(l4_cap_idx_t sem, l4_timeout_t timeout) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_semaphore_down_u(l4_cap_idx_t sem, l4_timeout_t to,
                    l4_utcb_t *utcb) L4_NOTHROW;


L4_INLINE l4_msgtag_t
l4_semaphore_down_u(l4_cap_idx_t sem, l4_timeout_t to,
                    l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_SEMAPHORE_OP_DOWN;
  return l4_ipc_call(sem, utcb, l4_msgtag(L4_PROTO_SEMAPHORE, 1, 0, 0), to);
}


L4_INLINE l4_msgtag_t
l4_semaphore_down(l4_cap_idx_t sem, l4_timeout_t to) L4_NOTHROW
{
  return l4_semaphore_down_u(sem, to, l4_utcb());
}

