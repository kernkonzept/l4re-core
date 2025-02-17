/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <l4/sys/types.h>
#include <sys/ucontext.h>
#include <ucontext.h>

/**
 * Architecture specific signal information.
 *
 * Some architectures store additional fault information. Only used in the
 * architecture specific parts.
 */
struct Sig_arch_context;

extern "C" {

/**
 * Trampoline function to jump to the signal handler.
 *
 * The details how the signal handler is called is architecture specific.
 * See ARCH-.../sig_trampoline.S for details. It is the responsibility of
 * the trampoline to jump to sigreturn() when the signal handler returns.
 */
void sigenter(void);

/**
 * Trampoline function to return from the signal handler.
 *
 * This function must trigger an exception IPC at the first instruction of
 * the function. The precise exception is irrelevant. The exception program
 * counter is used to detect that the signal frame should be unwound.
 */
void sigreturn(void);

} // extern "C"

/**
 * Adjust stack pointer to ABI constraints.
 */
static inline l4_addr_t sig_adjust_stack(l4_addr_t sp);

/**
 * Copy exception information into signal frame.
 *
 * \param[out] ucf   POSIX user context frame.
 * \param[in]  ue    L4 exception IPC register state
 * \param[in]  arch  Architecture specific signal state
 * \param[in]  pfa   Fault address
 */
static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue,
                         Sig_arch_context const &arch, l4_umword_t pfa);

/**
 * Restore exception IPC state from POSIX user context.
 *
 * This is used when the signal frame is unwound to restore the original thread
 * state.
 *
 * \param[out] ue   L4 exception IPC register state
 * \param[in]  ucf  POSIX user context frame
 */
static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf);

/**
 * Set up stack frame to enter signal handler.
 *
 * Needs to do all the architecture specific adjustments to the stack and
 * registers so that the signal handler is called. In particular, the stack
 * pointer must be set up in a way that `sp == ucf` holds at sigreturn().
 *
 * \param[in,out] ue   L4 exception IPC register state
 * \param[in]     ucf  POSIX user context frame.
 * \param[in]     si   POSIX signal information
 * \param[in]     sa   POSIX sigaction (mainly signal handler pointer)
 */
static inline
void setup_sighandler_frame(l4_exc_regs_t *ue, ucontext_t *ucf,
                            siginfo_t const *si, struct sigaction const &sa);

/**
 * Exception IPC to signal conversion result.
 */
enum class Exc_cause : int
{
  Unknown = -1, ///< An unknown L4 exception happened
  Ex_regs,      ///< Thread was interrupted by L4_THREAD_EX_REGS_TRIGGER_EXCEPTION
  Signal,       ///< The exception has been converted into a signal
};

/**
 * Convert L4 exception state to signal.
 *
 * Map an exception IPC state to a POSIX signal.
 *
 * \param[in]  regs  L4 exception IPC register state
 * \param[out] si    POSIX signal information
 * \param[out] arch  Architecture specific signal state
 *
 * \retval Exc_cause::Signal   The exception has been converted into a signal
 *                             (`si` and `arch` are valid).
 * \retval Exc_cause::Ex_regs  Thead interrupted. Pending signals may be
 *                             delivered.
 * \retval Exc_cause::Unknown  The L4 exception could not be converted.
 *                             (`si` and `arch` are *not* valid).
 */
static inline
Exc_cause map_exception_to_signal(l4_exc_regs_t const &regs, siginfo_t *si,
                                  Sig_arch_context *arch);

static inline
void dump_exception_state(L4Re::Util::Err const &err, l4_exc_regs_t const *r);
