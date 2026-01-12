#pragma once

#include <l4/sys/compiler.h>
#include "pthread-api.h"
#include <stdbool.h>

// TODO: TLS_TCB_AT_TP and TLS_DTV_AT_TP

L4_BEGIN_DECLS

/**
 * Callback that is invoked immediately before the creation of the first
 * additional thread via pthread_create().
 *
 * This gives the libc the chance to do any preparation necessary for
 * multi-threading (e.g. initializing locks).
 */
L4_HIDDEN int
ptlc_become_threaded(void);

/**
 * Allocate memory for TCB and TLS.
 *
 * \return Address the thread pointer shall be set to (without TLS_TCB_OFFSET).
 *         Maybe the return value should be of type `tcbhead_t *`?
 *
 * NOTE: The mem parameter was always NULL in our libpthread.
 * Original name: _dl_allocate_tls
 */
L4_HIDDEN void *
ptlc_allocate_tls(void);

/**
 * Deallocate memory for TCB and TLS.
 *
 * \param tls_tp  Address of the thread pointer (without TLS_TCB_OFFSET).
 *
 * NOTE: The dealloc_tcb parameter was always false in our libpthread.
 * Original name: _dl_deallocate_tls
 */
L4_HIDDEN void
ptlc_deallocate_tls(void *tls_tp);


// TOOD: We might want to get rid of this function, and instead just require
// that TLS gets inited before calling __pthread_initialize_minimal()...

/**
 * Unlike in the dynamically linked case the dynamic linker has not taken care
 * of initializing the TLS data structures.
 *
 * /param arg  As passed to __pthread_initialize_minimal().
 */
L4_HIDDEN void
ptlc_init_static_tls(void *arg);

/**
 * Setup TLS thread pointer.
 *
 * \param tls_tp  TLS TP pointer as returned by ptlc_allocate_tls().
 *
 * \retval  0  On success
 * \retval <0  On failure
 */
L4_HIDDEN int
ptlc_set_tp(void *tls_tp);

/**
 * Get pthread descriptor of the current thread.
 */
L4_HIDDEN pthread_descr
ptlc_thread_descr_self(void);

// TODO: Optimize, so make this a compile-time computation.

/**
 * Get pthread descriptor (struct pthread) from TLS TP.
 *
 * \param  tls_tp  TLS TP pointer as returned by ptlc_allocate_tls().
 *
 * \return The pthread description.
 */

L4_HIDDEN pthread_descr
ptlc_tls_tp_to_thread_descr(void *tls_tp);

L4_HIDDEN void *
ptlc_thread_descr_to_tls_tp(pthread_descr descr);

/**
 * Called before a thread is created, the thread has not yet been added to the
 * thread list.
 *
 * \note Executes on the thread that called pthread_create.
 */
L4_HIDDEN void
ptlc_before_create_thread(void);

/**
 * Called after a thread was created, after it was added to the thread list, and
 * might already have started executing.
 *
 * \param success  Whether thread creation was successful.
 *
 * \note Executes on the thread that called pthread_create.
 */
L4_HIDDEN void
ptlc_after_create_thread(bool success);

/**
 * Called when a thread exited, after it was removed from the thread list,
 * immedaitely before its thread descriptor freed.
 *
 * \note Executes on the pthread manager thread.
 */
L4_HIDDEN void
ptlc_after_exit_thread(void);

/**
 * Called at the end of ppthread initialization. Use to initialize libc
 * specific details, such as the dynamic loader of stdio locking.
 */
L4_HIDDEN void
ptlc_after_pthread_initialize(void);

L4_END_DECLS
