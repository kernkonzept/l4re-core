/**
 * \file
 * Priority Inheritance Mutex object.
 */
/*
 * Copyright (C) 2025-2026 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <l4/sys/types.h>
#include <l4/sys/utcb.h>

/**
 * \defgroup l4_pi_mutex_api Priority Inheritance Mutex C API
 * \{
 * \ingroup  l4_kernel_object_api
 *
 * C interface for controlling priority inheritance mutexes.
 *
 * \note PI mutexes are available only when kernel configuration option
 *       `PRIO_INHERIT` is enabled.
 *
 * A priority inheritance mutex (PI mutex) object provides the slow path for a
 * priority-inheritance-aware mutex. Here, priority inheritance means that the
 * mutex owner temporarily takes on the priority of the highest-priority waiting
 * thread, if that priority is higher than its own.
 *
 * A PI mutex can be used only by threads within a single task. This task is
 * defined when a PI mutex is created. Also, during creation, some kernel-user
 * memory of this task is reserved for storing the state of the mutex (see
 * L4::Factory::create_pi_mutex()). Using the PI mutex from another task results
 * in an error.
 *
 * A PI mutex object is intended to be used together with some logic in userland
 * for efficient locking and unlocking. As long as there is no contention on the
 * mutex, no syscall is needed for locking and unlocking. This is called the
 * *fast path*. Otherwise, the kernel interface needs to be used, which is
 * called the *slow path*.
 *
 * The kernel-user memory holding the mutex state facilitates the fast path and
 * determines when the slow path needs to be used. It consists of a single
 * machine word:
 *
 *      MSB    L4_CAP_SHIFT             0        bits
 *     ┌───────────────────┬─────┬─────────────┐
 *     │       owner       │ 0…0 │ waiter flag │
 *     └───────────────────┴─────┴─────────────┘
 *
 * When the whole word is zero, then the mutex is unlocked.
 *
 * For acquiring the mutex, userland first tries the fast path by trying to
 * update the status word via an atomic compare-and-swap operation: It tries to
 * replace the 0 (representing the unlocked state) with a capability index for
 * the current thread (i.e., the thread that is currently trying to acquire the
 * mutex). The capability index goes into the `owner` bits of the status word.
 * The `waiter flag` is left as 0. If the compare-and-swap succeeds, the mutex
 * was acquired successfully. If it fails, the mutex must be acquired via the
 * slow path, i.e., by calling l4_pi_mutex_lock(). This sets the `waiter flag`,
 * which signals that there was contention and that the slow path must also be
 * used for unlocking.
 *
 * Note that the capability index used for the owner in the mutex state must not
 * be zero. This is usually not an issue since %L4Re libraries reserve
 * capability index zero and do not use it for a thread capability.
 *
 * For releasing the mutex, userland first tries the fast path by trying the
 * reverse of the atomic compare-and-swap operation used in the fast path for
 * locking. If this succeeds, the mutex was released successfully. Otherwise,
 * i.e., if `waiter flag` is 1 or the `owner` differs from the current thread’s
 * capability index, the mutex must be released via the slow path, i.e., by
 * calling l4_pi_mutex_unlock().
 *
 * A minimal userland implementation in pseudo code:
 *
 * ```cpp
 * l4_ret_t userland_lock() {
 *   if (cas(status_ptr, 0, self_locked_state))
 *     return 0;  // success
 *   else
 *     return l4_error(l4_pi_mutex_lock(pm));
 * }
 *
 * l4_ret_t userland_unlock() {
 *   if (cas(status_ptr, self_locked_state, 0))
 *     return 0;  // success
 *   else
 *     return l4_error(l4_pi_mutex_unlock(pm));
 * }
 * ```
 *
 * In the above pseudo code,
 *
 * - `cas(ptr, old, new)` is an atomic compare-and-swap,
 *
 * - `status_ptr` is a pointer to the kernel-user memory holding the mutex
 *   state,
 *
 * - `self_locked_state` is a capability index for the current thread shifted
 *   into the `owner` bits of the state leaving `waiter flag` 0, and
 *
 * - `pm` is an #l4_cap_idx_t for the underlying PI mutex.
 *
 * \includefile{l4/sys/pi_mutex.h}
 */

/**
 * \ingroup l4_pi_mutex_api
 *
 * Lock contended priority inheritance mutex on slow path.
 *
 * \param mutex      Priority inheritance mutex object.
 * \param timeout    Timeout for blocking the mutex lock operation.
 *                   Note: The receive timeout of this timeout-pair is
 *                   significant for blocking, the send part is usually
 *                   non-blocking.
 *
 * \return Syscall return tag. Use l4_error() to check for errors.
 * \retval 0            Success.
 * \retval -L4_EAGAIN   Lock operation in kernel failed, can happen under
 *                      certain circumstances, try again.
 * \retval -L4_EDEADLK  Deadlock condition was detected or the calling thread
 *                      already owns the mutex.
 * \retval -L4_EINVAL   Invalid parameter or `ku_status` in invalid state.
 * \retval -L4_EPERM    The calling thread does not belong to the task the
 *                      mutex was bound to at creation.
 *
 * \retval L4_IPC_RETIMEOUT   Timeout expired before mutex could be locked.
 * \retval L4_IPC_RECANCELED  Thread was cancelled while waiting on the mutex.
 */
L4_INLINE l4_msgtag_t
l4_pi_mutex_lock(l4_cap_idx_t mutex, l4_timeout_t timeout) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_pi_mutex_lock_u(l4_cap_idx_t mutex, l4_timeout_t timeout,
                   l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_pi_mutex_api
 *
 * Unlock contended priority inheritance mutex from fast path or slow path.
 *
 * \param mutex      Priority inheritance mutex object.
 *
 * \return Syscall return tag. Use l4_error() to check for errors.
 * \retval 0            Success.
 * \retval -L4_EINVAL   Invalid parameter or `ku_status` in invalid state.
 * \retval -L4_EPERM    Calling thread was not the owner of the mutex or the
 *                      calling thread does not belong to the task the mutex
 *                      was bound to at creation.
 */
L4_INLINE l4_msgtag_t
l4_pi_mutex_unlock(l4_cap_idx_t mutex) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_pi_mutex_unlock_u(l4_cap_idx_t mutex, l4_utcb_t *utcb) L4_NOTHROW;

/**\} */ /* ends l4_pi_mutex_api group */


/**
 * \ingroup l4_protocol_ops
 *
 * Operations on priority inheritance mutex objects.
 *
 * See #L4_PROTO_PI_MUTEX for the protocol type to use for messages to
 * priority inheritance mutex objects.
 */
enum L4_pi_mutex_ops
{
  L4_PI_MUTEX_LOCK_OP      = 0UL, /**< Lock mutex */
  L4_PI_MUTEX_UNLOCK_OP    = 1UL, /**< Unlock mutex */
};

/* IMPLEMENTATION -----------------------------------------------------------*/

#include <l4/sys/ipc.h>

L4_INLINE l4_msgtag_t
l4_pi_mutex_lock_u(l4_cap_idx_t mutex, l4_timeout_t timeout,
                   l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_PI_MUTEX_LOCK_OP;
  return l4_ipc_call(mutex, utcb, l4_msgtag(L4_PROTO_PI_MUTEX, 1, 0, 0),
                     timeout);
}

L4_INLINE l4_msgtag_t
l4_pi_mutex_unlock_u(l4_cap_idx_t mutex, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  v->mr[0] = L4_PI_MUTEX_UNLOCK_OP;
  return l4_ipc_call(mutex, utcb, l4_msgtag(L4_PROTO_PI_MUTEX, 1, 0, 0),
                     L4_IPC_NEVER);
}


L4_INLINE l4_msgtag_t
l4_pi_mutex_lock(l4_cap_idx_t mutex, l4_timeout_t timeout) L4_NOTHROW
{
  return l4_pi_mutex_lock_u(mutex, timeout, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_pi_mutex_unlock(l4_cap_idx_t mutex) L4_NOTHROW
{
  return l4_pi_mutex_unlock_u(mutex, l4_utcb());
}
