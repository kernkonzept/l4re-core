/**
 * (c) 2015 Alexander Warg <alexander.warg@kernkonzept.com>
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

#include <l4/sys/irq.h>

enum L4_semaphore_op
{
  L4_SEMAPHORE_OP_DOWN    = 0,
  // semaphore up is IRQ_OP_TRIGGER with IRQ/Triggerable protocol
};

L4_INLINE l4_msgtag_t
l4_semaphore_up(l4_cap_idx_t sem) L4_NOTHROW
{
  return l4_irq_trigger(sem);
}

L4_INLINE l4_msgtag_t
l4_semaphore_up_u(l4_cap_idx_t sem, l4_utcb_t *utcb) L4_NOTHROW
{
  return l4_irq_trigger_u(sem, utcb);
}

L4_INLINE l4_msgtag_t
l4_semaphore_down(l4_cap_idx_t sem, l4_timeout_t to) L4_NOTHROW;

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

