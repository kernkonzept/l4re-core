/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
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
/**
 * \file
 * vCPU API
 */
#pragma once

#include <l4/sys/types.h>
#include <l4/sys/__vcpu-arch.h>

/**
 * \defgroup l4_vcpu_api vCPU API
 * \ingroup  l4_thread_api
 * vCPU API.
 *
 * The vCPU API in L4Re implements virtual processors (vCPUs) on top of
 * L4::Thread. This API can be used for user level threading, operating system
 * rehosting (see L4Linux) and virtualization.
 *
 * You switch a thread into `vCPU` operation with L4::Thread::vcpu_control.
 *
 * In vCPU mode, incoming IPC can be redirected to a handler function. If an
 * IPC is sent to the vCPU, the thread's normal execution is interrupted and the
 * handler called. Which kind of IPC is redirected is specified by the
 * #L4_vcpu_state_flags set in the l4_vcpu_state_t::state field of `vcpu_state`.
 * All events enabled in the `vcpu_state` field are redirected to the handler.
 * The handler is set via l4_vcpu_state_t::entry_ip and
 * l4_vcpu_state_t::entry_sp. IPC redirection works independent of "kernel"
 * and "user" mode, but see l4_vcpu_state_t::entry_sp. When the entry handler is
 * called, the UTCB contains the result of the IPC and content normally found
 * in CPU register is in l4_vcpu_state_t::i.
 *
 * Furthermore, the thread can execute in the context of different tasks,
 * called the "kernel" and the "user" mode. The kernel task is the one to which
 * the thread was originally bound via L4::Thread::control(). Execution starts
 * in the kernel task and it is always switched to when the asynchronous IPC
 * handler is invoked. When returning from the handler via
 * l4_thread_vcpu_resume_start() and l4_thread_vcpu_resume_commit(), a
 * different user task can be specified by setting l4_vcpu_state_t::user_task
 * and enabling the #L4_VCPU_F_USER_MODE flag in l4_vcpu_state_t::state.
 * Note that the kernel may cache the user task internally, see
 * l4_thread_vcpu_resume_commit().
 *
 * If the #L4_VCPU_F_USER_MODE flag is enabled, the following flags will be
 * automatically enabled in l4_vcpu_state_t::state on
 * L4::Thread::vcpu_resume_commit():
 * - #L4_VCPU_F_IRQ
 * - #L4_VCPU_F_PAGE_FAULTS
 * - #L4_VCPU_F_EXCEPTIONS
 *
 * When the kernel mode is entered, the following flags will be automatically
 * disabled in l4_vcpu_state_t::state:
 * - #L4_VCPU_F_IRQ
 * - #L4_VCPU_F_PAGE_FAULTS
 * - #L4_VCPU_F_USER_MODE
 *
 * Extended vCPU operation is used for hardware CPU virtualization. It can
 * be enabled with L4::Thread::vcpu_control_ext().
 *
 * \ref api_libvcpu defines a convenience API for working with vCPUs.
 *
 * \see api_libvcpu
 */

/**
 * State of a vCPU
 * \ingroup l4_vcpu_api
 */
typedef struct l4_vcpu_state_t
{
  l4_umword_t          version;       ///< vCPU ABI version. Set by the kernel
                                      ///< and must be checked by the user for
                                      ///< equality with #L4_VCPU_STATE_VERSION.
  l4_umword_t          user_data[7];  ///< User-specific data
  l4_vcpu_regs_t       r;             ///< Register state
  l4_vcpu_ipc_regs_t   i;             ///< IPC state

  l4_uint16_t          state;         ///< Current vCPU state. See #L4_vcpu_state_flags.
  l4_uint16_t          saved_state;   ///< Saved vCPU state. See #L4_vcpu_state_flags.
  l4_uint16_t          sticky_flags;  ///< Pending flags. See #L4_vcpu_sticky_flags.
  l4_uint16_t          _reserved;     ///< \internal

  l4_cap_idx_t         user_task;     ///< User task to use

  l4_umword_t          entry_sp;      ///< Stack pointer for entry (when coming from user task)
  l4_umword_t          entry_ip;      ///< IP for entry
  l4_umword_t          reserved_sp;   ///< \internal
  l4_vcpu_arch_state_t arch_state;    ///< Architecture-specific state
} l4_vcpu_state_t;

/**
 * State flags of a vCPU
 * \ingroup l4_vcpu_api
 */
enum L4_vcpu_state_flags
{
  /**
   * Receiving of IRQs and IPC enabled. While this flag is not set, the
   * corresponding vCPU thread will not receive any IPC and threads attempting
   * to send an IPC to this thread will block (according to the selected send
   * timeout).
   *
   * \note On L4::Thread::vcpu_resume_commit() this flag is automatically
   *       enabled in l4_vcpu_state_t::state if #L4_VCPU_F_USER_MODE is enabled.
   * \note When the kernel mode is entered, this flags is automatically
   *       disabled in l4_vcpu_state_t::state.
   */
  L4_VCPU_F_IRQ         = 0x01,

  /**
   * Page faults enabled. If this flag is set, a page fault switches to kernel
   * mode (potentially causing a VM exit) and calls the entry handler. If this
   * flag is not set, a page fault generates a page fault IPC to the pager of
   * the vCPU thread.
   *
   * \note IPC redirection for page faults controlled by this flag works
   *       independent of "kernel" and "user" mode.
   * \note On L4::Thread::vcpu_resume_commit() this flag is automatically
   *       enabled in l4_vcpu_state_t::state if #L4_VCPU_F_USER_MODE is enabled.
   * \note When the kernel mode is entered, this flags is automatically
   *       disabled in l4_vcpu_state_t::state.
   */
  L4_VCPU_F_PAGE_FAULTS = 0x02,

  /**
   * Exceptions enabled. If this flag is set, then, on the event of an
   * exception, the vCPU switches to kernel mode (potentially causing a VM
   * exit) and calls the entry handler. If this flag is not set, an exception
   * generates an exception IPC to the exception handler of the vCPU thread.
   *
   * \note IPC redirection for exceptions controlled by this flag works
   *       independent of "kernel" and "user" mode.
   * \note On L4::Thread::vcpu_resume_commit() this flag is automatically
   *       enabled in l4_vcpu_state_t::state if #L4_VCPU_F_USER_MODE is enabled.
   */
  L4_VCPU_F_EXCEPTIONS  = 0x04,

  /**
   * User task will be used. If set, the vCPU switches to user mode on next
   * L4::Thread::vcpu_resume_commit(). If clear, the vCPU stays in "kernel"
   * mode.
   *
   * \note When the kernel mode is entered, this flags is automatically
   *       disabled in l4_vcpu_state_t::state.
   */
  L4_VCPU_F_USER_MODE   = 0x20,

  /**
   * FPU enabled. This flag is only relevant if #L4_VCPU_F_USER_MODE is set.
   * Setting this flag allows code in vCPU mode to use the FPU. IF this flag
   * is not set, any FPU operation will trigger a corresponding exception
   * (FPU fault).
   */
  L4_VCPU_F_FPU_ENABLED = 0x80,
};

/**
 * Sticky flags of a vCPU
 * \ingroup l4_vcpu_api
 */
enum L4_vcpu_sticky_flags
{
  /// An event is pending: Either an IRQ or another thread attempts to send an
  /// IPC to this vCPU thread.
  L4_VCPU_SF_IRQ_PENDING = 0x01,
};

/**
 * Check if a vCPU state has the right version.
 *
 * \param  vcpu  A pointer to an initialized vCPU state.
 *
 * \retval 1  If the vCPU state has a matching version ID for the current
 *            vCPU user-level structures.
 * \retval 0  If the vCPU state has a different (incompatible) version ID than
 *            the current vCPU user-level structures.
 *
 */
L4_INLINE int
l4_vcpu_check_version(l4_vcpu_state_t const *vcpu) L4_NOTHROW;

/* IMPLEMENTATION: ------------------------------------------------*/

L4_INLINE int
l4_vcpu_check_version(l4_vcpu_state_t const *vcpu) L4_NOTHROW
{
  return vcpu->version == L4_VCPU_STATE_VERSION;
}
