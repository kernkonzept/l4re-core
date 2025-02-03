/**
 * \file
 * \brief ARM-specific thread related definitions.
 */
/*
 * (c) 2013 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include_next <l4/sys/thread.h>

// Use the full documentation from ARM64. Otherwise \parameters and \return
// would occur twice in the Doxygen documentation of this function.
/**
 * \brief Set the TPIDRURO thread specific register.
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

/**
 * \brief Arm specific L4::Thread::ex_regs() flags.
 * \ingroup l4_thread_api
 * \hideinitializer
 *
 * Only one option must be used in calls to L4::Thread::ex_regs(). Using more
 * than one option results in undefined behaviour.
 */
enum L4_thread_ex_regs_flags_arm
{
  /** Exception level set mask. */
  L4_THREAD_EX_REGS_ARM_SET_EL_MASK     = 0x3 << 24,
  /** Keep current exception level of thread (default). */
  L4_THREAD_EX_REGS_ARM_SET_EL_KEEP     = 0x0 << 24,
  /** Set exception level of thread to EL0 (usr mode). */
  L4_THREAD_EX_REGS_ARM_SET_EL_EL0      = 0x1 << 24,
  /** Set exception level of thread to EL1 (sys mode). */
  L4_THREAD_EX_REGS_ARM_SET_EL_EL1      = 0x2 << 24,
};

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
