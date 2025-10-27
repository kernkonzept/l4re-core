/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Frank Mehnert <frank.mehnert@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * Page fault trampoline handlers.
 */

#pragma once

#include <l4/sys/types.h>
#include <l4/sys/compiler.h>
#include <l4/sys/l4int.h>

/**
 * \defgroup l4_pf_trampoline_api Page fault trampoline handlers
 * \ingroup l4_api
 *
 * A page fault trampoline handler is an alternative mechanism to handle page
 * faults. Instead of sending a page fault IPC to the pager of the faulting
 * thread, a dedicated page fault handler is directly called by the kernel in
 * the context of the faulting thread.
 *
 * The handler has access to the instruction pointer and the page fault address.
 * The l4_pf_trampoline_t state contains this information plus additional
 * information saved by the kernel on entering the kernel page fault handler and
 * restored before leaving the kernel page fault handler.
 *
 * \includefile{l4/sys/pf_trampoline.h}
 */

/**
 * Definitions for l4_pf_trampoline_t::flags.
 * \ingroup l4_pf_trampoline_api
 */
enum L4_pf_trampoline_flags
{
  /**
   * The corresponding thread is currently handling a page fault using the
   * trampoline handler.
   *
   * The thread must not be ex-regs'd during that time!
   */
  Page_fault_in_progress = 1 << 0,
  /**
   * Trigger an exception at the corresponding thread when the thread resumes
   * execution after returning from a page fault trampoline handler.
   *
   * Used for deferred triggering of an artificial exception, for example, for
   * signal handling: If a thread resolving a page fault using a trampoline page
   * fault handler is being ex_regs'd(L4_THREAD_EX_REGS_TRIGGER_EXCEPTION), the
   * thread must not longjmp before the trampoline page fault handler resumed.
   * Instead, the signal handler just sets Trigger_exception_after_resume and
   * returns. Once the trampoline page fault handler resumes, the kernel resets
   * this flags and triggers the artificial exception which is again handled by
   * the signal handler.
   */
  Trigger_exception_after_resume = 1 << 1,
};

/**
 * Opaque type for the page fault trampoline register state.
 * \ingroup l4_pf_trampoline_api
 */
typedef struct l4_pf_trampoline_regs_t l4_pf_trampoline_regs_t;

/**
 * Opaque type for the page fault trampoline state.
 * \ingroup l4_pf_trampoline_api
 */
typedef struct l4_pf_trampoline_t l4_pf_trampoline_t;

#include <l4/sys/arch/pf_trampoline.h>


/**
 * State for page fault trampoline handler.
 * \ingroup l4_pf_trampoline_api
 */
typedef struct l4_pf_trampoline_t
{
  l4_umword_t   handler_ip;   /**< instruction pointer of trampoline handler */
  l4_umword_t   user_data[2]; /**< for userland maintenance */
  l4_umword_t   saved_mr0;    /**< MR[0] at entry of page fault handler */
  l4_umword_t   flags;        /**< See L4_pf_trampoline_flags */
  l4_pf_trampoline_regs_t state; /**< register state at entry of PF handler */
} l4_pf_trampoline_t;
