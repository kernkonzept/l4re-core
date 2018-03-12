/**
 * \file
 * \brief ARM-specific thread related definitions.
 */
/*
 * (c) 2013 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
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

#include_next <l4/sys/thread.h>

/**
 * \brief Set the TPIDRURO thread specific register.
 * \ingroup l4_thread_api
 *
 * \param thread   Thread to manipulate
 * \param tpidruro The value to be set
 * \return System call return tag
 */
L4_INLINE l4_msgtag_t
l4_thread_arm_set_tpidruro(l4_cap_idx_t thread, l4_addr_t tpidruro) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_thread_arm_set_tpidruro_u(l4_cap_idx_t thread, l4_addr_t tpidruro,
                             l4_utcb_t *utcb) L4_NOTHROW;

/* IMPLEMENTATION -----------------------------------------------------------*/

L4_INLINE l4_msgtag_t
l4_thread_arm_set_tpidruro_u(l4_cap_idx_t thread, l4_addr_t tpidruro,
                             l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_THREAD_ARM_TPIDRURO_OP;
  v->mr[1] = tpidruro;
  return l4_ipc_call(thread, utcb, l4_msgtag(L4_PROTO_THREAD, 2, 0, 0),
                     L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_arm_set_tpidruro(l4_cap_idx_t thread, l4_addr_t tpidruro) L4_NOTHROW
{
  return l4_thread_arm_set_tpidruro_u(thread, tpidruro, l4_utcb());
}
