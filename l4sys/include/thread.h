/**
 * \file
 * Common thread related definitions.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
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

#include <l4/sys/types.h>
#include <l4/sys/utcb.h>
#include <l4/sys/ipc.h>

/**
 * \defgroup l4_thread_api Thread
 * \ingroup  l4_kernel_object_api
 * Thread object.
 *
 * An L4 thread is a thread of execution in the L4 context.
 * Usually user-level and kernel threads are mapped 1:1 to each other.
 * Thread kernel objects are created using a factory, see \ref l4_factory_api
 * (l4_factory_create_thread()).
 *
 * Amongst other things an L4 thread encapsulates:
 * - CPU state
 *   - General-purpose registers
 *   - Program counter
 *   - Stack pointer
 * - FPU state
 * - Scheduling parameters, see the \ref l4_scheduler_api API
 * - Execution state
 *   - Blocked, Runnable, Running
 *
 * Thread objects provide an API for
 * - Thread configuration and manipulation
 * - Thread switching.
 *
 * The thread control functions are used to control various aspects of a
 * thread. See l4_thread_control_start() for more information.
 *
 * \includefile{l4/sys/thread.h}
 *
 * For the C++ interface refer to L4::Thread.
 */


/**
 * Exchange basic thread registers.
 * \ingroup l4_thread_api
 *
 * \param thread  Capability selector of the thread to manipulate.
 * \param ip      New instruction pointer, use ~0UL to leave the
 *                instruction pointer unchanged.
 * \param sp      New stack pointer, use ~0UL to leave the stack
 *                pointer unchanged.
 * \param flags   Ex-regs flags, see #L4_thread_ex_regs_flags.
 *
 * \return System call return tag
 *
 * This method allows to manipulate a thread. The basic functionality is to set
 * the instruction pointer and the stack pointer of a thread. Additionally,
 * this method allows also to cancel ongoing IPC operations and to force the
 * thread to raise an artificial exception (see `flags`).
 *
 * The thread is started using l4_scheduler_run_thread(). However, if at the
 * time l4_scheduler_run_thread() is called, the instruction pointer of the
 * thread is invalid, a later call to l4_thread_ex_regs() with a valid
 * instruction pointer might start the thread.
 */
L4_INLINE l4_msgtag_t
l4_thread_ex_regs(l4_cap_idx_t thread, l4_addr_t ip, l4_addr_t sp,
                  l4_umword_t flags) L4_NOTHROW;

/**
 * \ingroup l4_thread_api
 * \copybrief L4::Thread::ex_regs
 * \param thread  Capability selector of the thread to manipulate.
 * \copydetails L4::Thread::ex_regs
 */
L4_INLINE l4_msgtag_t
l4_thread_ex_regs_u(l4_cap_idx_t thread, l4_addr_t ip, l4_addr_t sp,
                    l4_umword_t flags, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Exchange basic thread registers and return previous values.
 * \ingroup l4_thread_api
 *
 * \param         thread  Capability selector of the thread to manipulate.
 * \param[in,out] ip      New instruction pointer, use ~0UL to leave the
 *                        instruction pointer unchanged, return previous
 *                        instruction pointer.
 * \param[in,out] sp      New stack pointer, use ~0UL to leave the stack
 *                        pointer unchanged, returns previous stack pointer.
 * \param[in,out] flags   Ex-regs flags, see #L4_thread_ex_regs_flags, return
 *                        previous CPU flags of the thread.
 *
 * \return System call return tag
 *
 * This method allows to manipulate and start a thread. The basic
 * functionality is to set the instruction pointer and the stack pointer of a
 * thread. Additionally, this method allows also to cancel ongoing IPC
 * operations and to force the thread to raise an artificial exception (see
 * `flags`).
 *
 * Returned values are valid only if function returns successfully.
 */
L4_INLINE l4_msgtag_t
l4_thread_ex_regs_ret(l4_cap_idx_t thread, l4_addr_t *ip, l4_addr_t *sp,
                      l4_umword_t *flags) L4_NOTHROW;

/**
 * \ingroup l4_thread_api
 * \copybrief L4::Thread::ex_regs(l4_addr_t*,l4_addr_t*,l4_umword_t*,l4_utcb_t*)
 * \param thread  Capability selector of the thread to manipulate.
 * \copydetails L4::Thread::ex_regs(l4_addr_t*,l4_addr_t*,l4_umword_t*,l4_utcb_t*)
 */
L4_INLINE l4_msgtag_t
l4_thread_ex_regs_ret_u(l4_cap_idx_t thread, l4_addr_t *ip, l4_addr_t *sp,
                        l4_umword_t *flags, l4_utcb_t *utcb) L4_NOTHROW;



/**
 * \defgroup l4_thread_control_api Thread control
 * \ingroup l4_thread_api
 *
 * API for Thread Control method.
 *
 *
 * The thread control API provides access to almost any parameter of a thread
 * object. The API is based on a single invocation of the thread object.
 * However, because of the huge amount of parameters, the API provides a set
 * of functions to set specific parameters of a thread and a commit function
 * to commit the thread control call (see l4_thread_control_commit()).
 *
 * A thread control operation must always start with l4_thread_control_start()
 * and be committed with l4_thread_control_commit().  All other thread control
 * parameter setter functions must be called between these two functions.
 *
 * An example for a sequence of thread control API calls can be found below.
 *
 *  l4_utcb_t *u = l4_utcb(); <br>
 *  \link l4_thread_control_start() l4_thread_control_start(u)\endlink; <br>
 *  \link l4_thread_control_pager() l4_thread_control_pager(u, pager_cap)\endlink; <br>
 *  \link l4_thread_control_bind() l4_thread_control_bind (u, thread_utcb, task)\endlink; <br>
 *  \link l4_thread_control_commit() l4_thread_control_commit(u, thread_cap)\endlink; <br>
 *
 */

/**
 * Start a thread control API sequence.
 * \ingroup l4_thread_control_api
 *
 * This function starts a sequence of thread control API functions.
 * After this functions any of following functions may be called in any order.
 * - l4_thread_control_pager()
 * - l4_thread_control_exc_handler()
 * - l4_thread_control_bind()
 * - l4_thread_control_alien()
 * - l4_thread_control_ux_host_syscall() (Fiasco-UX only)
 *
 * To commit the changes to the thread l4_thread_control_commit() must be
 * called in the end.
 *
 * \note The thread control API calls store the parameters for the thread in
 *       the UTCB of the caller, this means between l4_thread_control_start()
 *       and l4_thread_control_commit() no functions that modify the UTCB
 *       contents must be called.
 */
L4_INLINE void
l4_thread_control_start(void) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_control_api
 */
L4_INLINE void
l4_thread_control_start_u(l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Set the pager.
 * \ingroup l4_thread_control_api
 *
 * \param pager     Capability selector invoked to send a page-fault IPC.
 *
 * \note The pager capability selector is interpreted in the task the thread
 *       is bound to (executes in).
 */
L4_INLINE void
l4_thread_control_pager(l4_cap_idx_t pager) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_control_api
 */
L4_INLINE void
l4_thread_control_pager_u(l4_cap_idx_t pager, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Set the exception handler.
 * \ingroup l4_thread_control_api
 *
 * \param exc_handler  Capability selector invoked to send an exception IPC.
 *
 * \note The exception-handler capability selector is interpreted in the task
 *       the thread is bound to (executes in).
 */
L4_INLINE void
l4_thread_control_exc_handler(l4_cap_idx_t exc_handler) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_control_api
 */
L4_INLINE void
l4_thread_control_exc_handler_u(l4_cap_idx_t exc_handler,
                                l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Bind the thread to a task.
 * \ingroup l4_thread_control_api
 *
 * \param thread_utcb  The address of the UTCB in the target task.
 * \param task         The target task of the thread.
 *
 * Binding a thread to a task has the effect that the thread
 * afterwards executes code within that task and has access to the
 * resources visible within that task.
 *
 * \note There should not be more than one thread use a UTCB to prevent
 *       data corruption.
 *
 */
L4_INLINE void
l4_thread_control_bind(l4_utcb_t *thread_utcb,
                       l4_cap_idx_t task) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_control_api
 */
L4_INLINE void
l4_thread_control_bind_u(l4_utcb_t *thread_utcb,
                         l4_cap_idx_t task, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Enable alien mode.
 * \ingroup l4_thread_control_api
 * \param   on    Boolean value defining the state of the feature.
 *
 * Alien mode means the thread is not allowed to invoke L4 kernel objects
 * directly and it is also not allowed to allocate FPU state. All those
 * operations result in an exception IPC that gets sent through the pager
 * capability. The responsible pager can then selectively allow an object
 * invocation or allocate FPU state for the thread.
 *
 * This feature can be used to attach a debugger to a thread and trace all
 * object invocations.
 */
L4_INLINE void
l4_thread_control_alien(int on) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_control_api
 */
L4_INLINE void
l4_thread_control_alien_u(l4_utcb_t *utcb, int on) L4_NOTHROW;

/**
 * Enable pass through of native host (Linux) system calls.
 * \ingroup l4_thread_control_api
 * \param   on    Boolean value defining the state of the feature.
 *
 * \pre Running on Fiasco-UX
 *
 * This enables the thread to do host system calls. This feature is only
 * available in Fiasco-UX and ignored in other environments.
 */
L4_INLINE void
l4_thread_control_ux_host_syscall(int on) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_control_api
 */
L4_INLINE void
l4_thread_control_ux_host_syscall_u(l4_utcb_t *utcb, int on) L4_NOTHROW;



/**
 * Commit the thread control parameters.
 * \ingroup l4_thread_control_api
 *
 * \param thread  Capability selector of target thread to commit to.
 * \return system call return tag
 */
L4_INLINE l4_msgtag_t
l4_thread_control_commit(l4_cap_idx_t thread) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_control_api
 */
L4_INLINE l4_msgtag_t
l4_thread_control_commit_u(l4_cap_idx_t thread, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Yield current time slice.
 * \ingroup l4_thread_api
 *
 * \return system call return tag
 */
L4_INLINE l4_msgtag_t
l4_thread_yield(void) L4_NOTHROW;

/**
 * Switch to another thread (and donate the remaining time slice).
 * \ingroup l4_thread_api
 *
 * \param to_thread   The thread to switch to.
 *
 * \return system call return tag
 */
L4_INLINE l4_msgtag_t
l4_thread_switch(l4_cap_idx_t to_thread) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_thread_switch_u(l4_cap_idx_t to_thread, l4_utcb_t *utcb) L4_NOTHROW;



/**
 * Get consumed time of thread in µs.
 * \ingroup l4_thread_api
 *
 * \param      thread  Thread to get the consumed time from.
 * \param[out] us      Consumed time in µs.
 *
 * \return system call return tag
 */
L4_INLINE l4_msgtag_t
l4_thread_stats_time(l4_cap_idx_t thread, l4_kernel_clock_t *us) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_thread_stats_time_u(l4_cap_idx_t thread, l4_kernel_clock_t *us,
                       l4_utcb_t *utcb) L4_NOTHROW;


/**
 * vCPU return from event handler.
 * \ingroup l4_thread_api
 *
 * \return Message tag to be used for l4_sndfpage_add() and
 *         l4_thread_vcpu_resume_commit()
 *
 * The vCPU resume functionality is split in multiple functions to allow the
 * specification of additional send-flex-pages using l4_sndfpage_add().
 */
L4_INLINE l4_msgtag_t
l4_thread_vcpu_resume_start(void) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_thread_vcpu_resume_start_u(l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Commit vCPU resume.
 * \ingroup l4_thread_api
 *
 * \param thread    Thread to be resumed, the invalid cap can be used
 *                  for the current thread.
 * \param tag       Tag to use, returned by l4_thread_vcpu_resume_start()
 *
 * \return System call result message tag. In extended vCPU mode and when
 * the virtual interrupts are cleared, the return code 1 flags an incoming
 * IPC message, whereas 0 indicates a VM exit. An error is returned upon:
 *   - Insufficient rights on the given task capability (-L4_EPERM).
 *   - Given task capability is invalid (-L4_ENOENT).
 *   - A supplied mapping failed.
 *
 * To resume into another address space the capability to the target task
 * must be set in the vCPU-state, with all lower bits in the task
 * capability cleared. The kernel adds the #L4_SYSF_SEND flag to this field
 * to indicate that the capability has been referenced in the kernel.
 * Consecutive resumes will not reference the task capability again until
 * all bits are cleared again. To release a task use the different task
 * capability or use an invalid capability with the #L4_SYSF_REPLY flag set.
 *
 * \see l4_vcpu_state_t
 */
L4_INLINE l4_msgtag_t
l4_thread_vcpu_resume_commit(l4_cap_idx_t thread,
                             l4_msgtag_t tag) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_thread_vcpu_resume_commit_u(l4_cap_idx_t thread,
                               l4_msgtag_t tag, l4_utcb_t *utcb) L4_NOTHROW;


/**
 * Enable or disable the vCPU feature for the thread.
 * \ingroup l4_thread_api
 *
 * \param thread      Capability selector of the thread for which the vCPU
 *                    feature shall be enabled or disabled.
 * \param vcpu_state  The virtual address where the kernel shall store the vCPU
 *                    state in case of vCPU exits. The address must be a valid
 *                    kernel-user-memory address (see l4_task_add_ku_mem()).
 *
 * \return Syscall return tag.
 *
 * This function enables the vCPU feature of the `thread` if `vcpu_state`
 * is set to a valid kernel-user-memory address, or disables the vCPU feature
 * if `vcpu_state` is 0.
 *
 */
L4_INLINE l4_msgtag_t
l4_thread_vcpu_control(l4_cap_idx_t thread, l4_addr_t vcpu_state) L4_NOTHROW;

/**
 * \ingroup l4_thread_api
 * \copybrief L4::Thread::vcpu_control
 * \param thread  Capability selector of the thread for which the vCPU feature
 *                shall be enabled or disabled.
 * \copydetails L4::Thread::vcpu_control
 */
L4_INLINE l4_msgtag_t
l4_thread_vcpu_control_u(l4_cap_idx_t thread, l4_addr_t vcpu_state,
                         l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Enable or disable the extended vCPU feature for the thread.
 * \ingroup l4_thread_api
 *
 * \param thread          Capability selector of the thread for which the
 *                        extended vCPU feature shall be enabled or disabled.
 * \param ext_vcpu_state  The virtual address where the kernel shall store the
 *                        vCPU state in case of vCPU exits. The address must be
 *                        a valid kernel-user-memory address (see
 *                        l4_task_add_ku_mem()).
 *
 * \return Systemcall result message tag.
 *
 * The extended vCPU feature allows the use of hardware-virtualization
 * features such as Intel's VT or AMD's SVM.
 *
 * This function enables the extended vCPU feature of the `thread`
 * if `ext_vcpu_state` is set to a valid kernel-user-memory address, or disables
 * the vCPU feature if `ext_vcpu_state` is 0.
 *
 * \note The extended vCPU mode includes the normal vCPU mode.
 */
L4_INLINE l4_msgtag_t
l4_thread_vcpu_control_ext(l4_cap_idx_t thread, l4_addr_t ext_vcpu_state) L4_NOTHROW;

/**
 * \ingroup l4_thread_api
 * \copybrief L4::Thread::vcpu_control_ext
 * \param thread  Capability selector of the thread for which the extended vCPU
 *                feature shall be enabled or disabled.
 * \copydetails L4::Thread::vcpu_control_ext
 */
L4_INLINE l4_msgtag_t
l4_thread_vcpu_control_ext_u(l4_cap_idx_t thread, l4_addr_t ext_vcpu_state,
                             l4_utcb_t *utcb) L4_NOTHROW;


/**
 * \copybrief L4::Thread::register_del_irq
 * \ingroup l4_thread_api
 *
 * \param thread  Thread to register IRQ for.
 * \param irq     Capability selector for the IRQ object to be triggered.
 *
 * \return System call return tag containing the return code.
 *
 * An example of a deletion event is the removal of an IPC gate
 * that is bound to this thread.
 */
L4_INLINE l4_msgtag_t
l4_thread_register_del_irq(l4_cap_idx_t thread, l4_cap_idx_t irq) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_thread_register_del_irq_u(l4_cap_idx_t thread, l4_cap_idx_t irq,
                             l4_utcb_t *utcb) L4_NOTHROW;

/**
 * Start a thread sender modifiction sequence.
 * \ingroup l4_thread_api
 *
 * Add modification rules with l4_thread_modify_sender_add() and commit with
 * l4_thread_modify_sender_commit(). Do not touch the UTCB between
 * l4_thread_modify_sender_start() and l4_thread_modify_sender_commit().
 *
 * \see l4_thread_modify_sender_add
 * \see l4_thread_modify_sender_commit
 */
L4_INLINE l4_msgtag_t
l4_thread_modify_sender_start(void) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_thread_modify_sender_start_u(l4_utcb_t *u) L4_NOTHROW;

/**
 * Add a modification pattern to a sender modification sequence.
 * \ingroup l4_thread_api
 *
 * \param tag         Tag received from l4_thread_modify_sender_start() or
 *                    previous l4_thread_modify_sender_add() calls from
 *                    the same sequence.
 * \param match_mask  Bitmask of bits to match the label.
 * \param match       Bitmask that must be equal to the label after applying
 *                    match_mask.
 * \param del_bits    Bits to be deleted from the label.
 * \param add_bits    Bits to be added to the label.
 *
 * \return 0 on sucess, <0 on error
 *
 * In pseudo code:
 *   if ((sender_label & match_mask) == match)
 *     { label = (label & ~del_bits) | add_bits; }
 *
 * Only the first match is applied.
 *
 * \see l4_thread_modify_sender_start
 * \see l4_thread_modify_sender_commit
 */
L4_INLINE int
l4_thread_modify_sender_add(l4_umword_t match_mask,
                            l4_umword_t match,
                            l4_umword_t del_bits,
                            l4_umword_t add_bits,
                            l4_msgtag_t *tag) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_api
 */
L4_INLINE int
l4_thread_modify_sender_add_u(l4_umword_t match_mask,
                              l4_umword_t match,
                              l4_umword_t del_bits,
                              l4_umword_t add_bits,
                              l4_msgtag_t *tag, l4_utcb_t *u) L4_NOTHROW;

/**
 * Apply (commit) a sender modification sequence.
 * \ingroup l4_thread_api
 *
 * \see l4_thread_modify_sender_start
 * \see l4_thread_modify_sender_add
 */
L4_INLINE l4_msgtag_t
l4_thread_modify_sender_commit(l4_cap_idx_t thread, l4_msgtag_t tag) L4_NOTHROW;

/**
 * \internal
 * \ingroup l4_thread_api
 */
L4_INLINE l4_msgtag_t
l4_thread_modify_sender_commit_u(l4_cap_idx_t thread, l4_msgtag_t tag,
                                 l4_utcb_t *u) L4_NOTHROW;

/**
 * Operations on thread objects.
 * \ingroup l4_protocol_ops
 * \hideinitializer
 * \internal
 */
enum L4_thread_ops
{
  L4_THREAD_CONTROL_OP                = 0UL,    /**< Control operation */
  L4_THREAD_EX_REGS_OP                = 1UL,    /**< Exchange registers operation */
  L4_THREAD_SWITCH_OP                 = 2UL,    /**< Do a thread switch */
  L4_THREAD_STATS_OP                  = 3UL,    /**< Thread statistics */
  L4_THREAD_VCPU_RESUME_OP            = 4UL,    /**< VCPU resume */
  L4_THREAD_REGISTER_DELETE_IRQ_OP    = 5UL,    /**< Register an IPC-gate deletion IRQ */
  L4_THREAD_MODIFY_SENDER_OP          = 6UL,    /**< Modify all senders IDs that match the given pattern */
  L4_THREAD_VCPU_CONTROL_OP           = 7UL,    /**< Enable / disable VCPU feature */
  L4_THREAD_VCPU_CONTROL_EXT_OP       = L4_THREAD_VCPU_CONTROL_OP | 0x10000,
  L4_THREAD_X86_GDT_OP                = 0x10UL, /**< Gdt */
  L4_THREAD_ARM_TPIDRURO_OP           = 0x10UL, /**< Set TPIDRURO register */
  L4_THREAD_AMD64_SET_SEGMENT_BASE_OP = 0x12UL, /**< Set segment base */
  L4_THREAD_OPCODE_MASK               = 0xffff, /**< Mask for opcodes */
};

/**
 * Flags for the thread control operation.
 * \ingroup l4_thread_api
 * \hideinitializer
 * \internal
 *
 * Values for the enabled flags need to be given in their appropriate field
 * in the UTCB,
 * \see l4_thread_control
 */
enum L4_thread_control_flags
{
  /** The pager will be given. */
  L4_THREAD_CONTROL_SET_PAGER       = 0x0010000,
  /** The task to bind the thread to will be given. */
  L4_THREAD_CONTROL_BIND_TASK       = 0x0200000,
  /** Alien state of the thread is set. */
  L4_THREAD_CONTROL_ALIEN           = 0x0400000,
  /** Fiasco-UX only: pass-through of host system calls is set. */
  L4_THREAD_CONTROL_UX_NATIVE       = 0x0800000,
  /** The exception handler of the thread will be given. */
  L4_THREAD_CONTROL_SET_EXC_HANDLER = 0x1000000,
};

/**
 * Indices for the values in the message register for thread control.
 * \ingroup l4_thread_api
 * \hideinitializer
 * \internal
 *
 * The values indicate the index in the message registers during
 * thread-control operation.
 */
enum L4_thread_control_mr_indices
{
  L4_THREAD_CONTROL_MR_IDX_FLAGS       = 0, /**< \see #L4_thread_control_flags. */
  L4_THREAD_CONTROL_MR_IDX_PAGER       = 1, /**< Index for pager cap */
  L4_THREAD_CONTROL_MR_IDX_EXC_HANDLER = 2, /**< Index for exception handler */
  L4_THREAD_CONTROL_MR_IDX_FLAG_VALS   = 4, /**< Index for feature values */
  L4_THREAD_CONTROL_MR_IDX_BIND_UTCB   = 5, /**< Index for UTCB address for bind */
  L4_THREAD_CONTROL_MR_IDX_BIND_TASK   = 6, /**< Index for task flex-page for bind */
};

/**
 * Flags for the thread ex-regs operation.
 * \ingroup l4_thread_api
 * \hideinitializer
 */
enum L4_thread_ex_regs_flags
{
  L4_THREAD_EX_REGS_CANCEL            = 0x10000UL, /**< Cancel ongoing IPC in the thread. */
  L4_THREAD_EX_REGS_TRIGGER_EXCEPTION = 0x20000UL, /**< Trigger artificial exception in thread. */
};


/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>
#include <l4/sys/types.h>

L4_INLINE l4_msgtag_t
l4_thread_ex_regs_u(l4_cap_idx_t thread, l4_addr_t ip, l4_addr_t sp,
                    l4_umword_t flags, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_THREAD_EX_REGS_OP | flags;
  v->mr[1] = ip;
  v->mr[2] = sp;
  return l4_ipc_call(thread, utcb, l4_msgtag(L4_PROTO_THREAD, 3, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_ex_regs_ret_u(l4_cap_idx_t thread, l4_addr_t *ip, l4_addr_t *sp,
                        l4_umword_t *flags, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  l4_msgtag_t ret = l4_thread_ex_regs_u(thread, *ip, *sp, *flags, utcb);
  if (l4_error_u(ret, utcb))
    return ret;

  *flags = v->mr[0];
  *ip    = v->mr[1];
  *sp    = v->mr[2];
  return ret;
}

L4_INLINE void
l4_thread_control_start_u(l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[L4_THREAD_CONTROL_MR_IDX_FLAGS] = L4_THREAD_CONTROL_OP;
}

L4_INLINE void
l4_thread_control_pager_u(l4_cap_idx_t pager, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[L4_THREAD_CONTROL_MR_IDX_FLAGS] |= L4_THREAD_CONTROL_SET_PAGER;
  v->mr[L4_THREAD_CONTROL_MR_IDX_PAGER]  = pager;
}

L4_INLINE void
l4_thread_control_exc_handler_u(l4_cap_idx_t exc_handler,
                                l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[L4_THREAD_CONTROL_MR_IDX_FLAGS]       |= L4_THREAD_CONTROL_SET_EXC_HANDLER;
  v->mr[L4_THREAD_CONTROL_MR_IDX_EXC_HANDLER]  = exc_handler;
}

L4_INLINE void
l4_thread_control_bind_u(l4_utcb_t *thread_utcb, l4_cap_idx_t task,
                         l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[L4_THREAD_CONTROL_MR_IDX_FLAGS]         |= L4_THREAD_CONTROL_BIND_TASK;
  v->mr[L4_THREAD_CONTROL_MR_IDX_BIND_UTCB]      = (l4_addr_t)thread_utcb;
  v->mr[L4_THREAD_CONTROL_MR_IDX_BIND_TASK]      = L4_ITEM_MAP;
  v->mr[L4_THREAD_CONTROL_MR_IDX_BIND_TASK + 1]  = l4_obj_fpage(task, 0, L4_FPAGE_RWX).raw;
}

L4_INLINE void
l4_thread_control_alien_u(l4_utcb_t *utcb, int on) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[L4_THREAD_CONTROL_MR_IDX_FLAGS]     |= L4_THREAD_CONTROL_ALIEN;
  v->mr[L4_THREAD_CONTROL_MR_IDX_FLAG_VALS] |= on ? L4_THREAD_CONTROL_ALIEN : 0;
}

L4_INLINE void
l4_thread_control_ux_host_syscall_u(l4_utcb_t *utcb, int on) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[L4_THREAD_CONTROL_MR_IDX_FLAGS]     |= L4_THREAD_CONTROL_UX_NATIVE;
  v->mr[L4_THREAD_CONTROL_MR_IDX_FLAG_VALS] |= on ? L4_THREAD_CONTROL_UX_NATIVE : 0;
}

L4_INLINE l4_msgtag_t
l4_thread_control_commit_u(l4_cap_idx_t thread, l4_utcb_t *utcb) L4_NOTHROW
{
  int items = 0;
  if (l4_utcb_mr_u(utcb)->mr[L4_THREAD_CONTROL_MR_IDX_FLAGS] & L4_THREAD_CONTROL_BIND_TASK)
    items = 1;
  return l4_ipc_call(thread, utcb, l4_msgtag(L4_PROTO_THREAD, 6, items, 0), L4_IPC_NEVER);
}


L4_INLINE l4_msgtag_t
l4_thread_yield(void) L4_NOTHROW
{
  l4_ipc_receive(L4_INVALID_CAP, NULL, L4_IPC_BOTH_TIMEOUT_0);
  return l4_msgtag(0, 0, 0, 0);
}

/* Preliminary, to be changed */
L4_INLINE l4_msgtag_t
l4_thread_switch_u(l4_cap_idx_t to_thread, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_THREAD_SWITCH_OP;
  return l4_ipc_call(to_thread, utcb, l4_msgtag(L4_PROTO_THREAD, 1, 0, 0), L4_IPC_NEVER);
}


L4_INLINE l4_msgtag_t
l4_thread_stats_time_u(l4_cap_idx_t thread, l4_kernel_clock_t *us,
                       l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  l4_msgtag_t res;

  v->mr[0] = L4_THREAD_STATS_OP;

  res = l4_ipc_call(thread, utcb, l4_msgtag(L4_PROTO_THREAD, 1, 0, 0), L4_IPC_NEVER);

  if (l4_msgtag_has_error(res))
    return res;

  *us = v->mr64[l4_utcb_mr64_idx(0)];

  return res;
}

L4_INLINE l4_msgtag_t
l4_thread_vcpu_resume_start_u(l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_THREAD_VCPU_RESUME_OP;
  return l4_msgtag(L4_PROTO_THREAD, 1, 0, 0);
}

L4_INLINE l4_msgtag_t
l4_thread_vcpu_resume_commit_u(l4_cap_idx_t thread,
                               l4_msgtag_t tag, l4_utcb_t *utcb) L4_NOTHROW
{
  return l4_ipc_call(thread, utcb, tag, L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_ex_regs(l4_cap_idx_t thread, l4_addr_t ip, l4_addr_t sp,
                    l4_umword_t flags) L4_NOTHROW
{
  return l4_thread_ex_regs_u(thread, ip, sp, flags, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_thread_ex_regs_ret(l4_cap_idx_t thread, l4_addr_t *ip, l4_addr_t *sp,
                      l4_umword_t *flags) L4_NOTHROW
{
  return l4_thread_ex_regs_ret_u(thread, ip, sp, flags, l4_utcb());
}

L4_INLINE void
l4_thread_control_start(void) L4_NOTHROW
{
  l4_thread_control_start_u(l4_utcb());
}

L4_INLINE void
l4_thread_control_pager(l4_cap_idx_t pager) L4_NOTHROW
{
  l4_thread_control_pager_u(pager, l4_utcb());
}

L4_INLINE void
l4_thread_control_exc_handler(l4_cap_idx_t exc_handler) L4_NOTHROW
{
  l4_thread_control_exc_handler_u(exc_handler, l4_utcb());
}


L4_INLINE void
l4_thread_control_bind(l4_utcb_t *thread_utcb, l4_cap_idx_t task) L4_NOTHROW
{
  l4_thread_control_bind_u(thread_utcb, task, l4_utcb());
}

L4_INLINE void
l4_thread_control_alien(int on) L4_NOTHROW
{
  l4_thread_control_alien_u(l4_utcb(), on);
}

L4_INLINE void
l4_thread_control_ux_host_syscall(int on) L4_NOTHROW
{
  l4_thread_control_ux_host_syscall_u(l4_utcb(), on);
}

L4_INLINE l4_msgtag_t
l4_thread_control_commit(l4_cap_idx_t thread) L4_NOTHROW
{
  return l4_thread_control_commit_u(thread, l4_utcb());
}




L4_INLINE l4_msgtag_t
l4_thread_switch(l4_cap_idx_t to_thread) L4_NOTHROW
{
  return l4_thread_switch_u(to_thread, l4_utcb());
}




L4_INLINE l4_msgtag_t
l4_thread_stats_time(l4_cap_idx_t thread, l4_kernel_clock_t *us) L4_NOTHROW
{
  return l4_thread_stats_time_u(thread, us, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_thread_vcpu_resume_start(void) L4_NOTHROW
{
  return l4_thread_vcpu_resume_start_u(l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_thread_vcpu_resume_commit(l4_cap_idx_t thread,
                             l4_msgtag_t tag) L4_NOTHROW
{
  return l4_thread_vcpu_resume_commit_u(thread, tag, l4_utcb());
}


L4_INLINE l4_msgtag_t
l4_thread_register_del_irq_u(l4_cap_idx_t thread, l4_cap_idx_t irq,
                             l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(u);
  m->mr[0] = L4_THREAD_REGISTER_DELETE_IRQ_OP;
  m->mr[1] = l4_map_obj_control(0,0);
  m->mr[2] = l4_obj_fpage(irq, 0, L4_CAP_FPAGE_RWS).raw;
  return l4_ipc_call(thread, u, l4_msgtag(L4_PROTO_THREAD, 1, 1, 0), L4_IPC_NEVER);

}

L4_INLINE l4_msgtag_t
l4_thread_register_del_irq(l4_cap_idx_t thread, l4_cap_idx_t irq) L4_NOTHROW
{
  return l4_thread_register_del_irq_u(thread, irq, l4_utcb());
}


L4_INLINE l4_msgtag_t
l4_thread_vcpu_control_u(l4_cap_idx_t thread, l4_addr_t vcpu_state,
                         l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_THREAD_VCPU_CONTROL_OP;
  v->mr[1] = vcpu_state;
  return l4_ipc_call(thread, utcb, l4_msgtag(L4_PROTO_THREAD, 2, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_vcpu_control(l4_cap_idx_t thread, l4_addr_t vcpu_state) L4_NOTHROW
{ return l4_thread_vcpu_control_u(thread, vcpu_state, l4_utcb()); }


L4_INLINE l4_msgtag_t
l4_thread_vcpu_control_ext_u(l4_cap_idx_t thread, l4_addr_t ext_vcpu_state,
                             l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_THREAD_VCPU_CONTROL_EXT_OP;
  v->mr[1] = ext_vcpu_state;
  return l4_ipc_call(thread, utcb, l4_msgtag(L4_PROTO_THREAD, 2, 0, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_vcpu_control_ext(l4_cap_idx_t thread, l4_addr_t ext_vcpu_state) L4_NOTHROW
{ return l4_thread_vcpu_control_ext_u(thread, ext_vcpu_state, l4_utcb()); }

L4_INLINE l4_msgtag_t
l4_thread_modify_sender_start_u(l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(u);
  m->mr[0] = L4_THREAD_MODIFY_SENDER_OP;
  return l4_msgtag(L4_PROTO_THREAD, 1, 0, 0);
}

L4_INLINE int
l4_thread_modify_sender_add_u(l4_umword_t match_mask,
                              l4_umword_t match,
                              l4_umword_t del_bits,
                              l4_umword_t add_bits,
                              l4_msgtag_t *tag, l4_utcb_t *u) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(u);
  unsigned w = l4_msgtag_words(*tag);
  if (w >= L4_UTCB_GENERIC_DATA_SIZE - 4)
    return -L4_ENOMEM;

  m->mr[w]   = match_mask;
  m->mr[w+1] = match;
  m->mr[w+2] = del_bits;
  m->mr[w+3] = add_bits;

  *tag = l4_msgtag(l4_msgtag_label(*tag), w + 4, 0, 0);

  return 0;
}

L4_INLINE l4_msgtag_t
l4_thread_modify_sender_commit_u(l4_cap_idx_t thread, l4_msgtag_t tag,
                                 l4_utcb_t *u) L4_NOTHROW
{
  return l4_ipc_call(thread, u, tag, L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_thread_modify_sender_start(void) L4_NOTHROW
{
  return l4_thread_modify_sender_start_u(l4_utcb());
}

L4_INLINE int
l4_thread_modify_sender_add(l4_umword_t match_mask,
                            l4_umword_t match,
                            l4_umword_t del_bits,
                            l4_umword_t add_bits,
                            l4_msgtag_t *tag) L4_NOTHROW
{
  return l4_thread_modify_sender_add_u(match_mask, match,
                                       del_bits, add_bits, tag, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_thread_modify_sender_commit(l4_cap_idx_t thread, l4_msgtag_t tag) L4_NOTHROW
{
  return l4_thread_modify_sender_commit_u(thread, tag, l4_utcb());
}
