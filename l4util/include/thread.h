/**
 * \file
 * \brief  Low-level Thread Functions
 *
 * \date   1997
 * \author Sebastian Schönberg */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef __L4_THREAD_H
#define __L4_THREAD_H

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <l4/sys/scheduler.h>

L4_BEGIN_DECLS

/**
 * \defgroup l4util_thread Low-Level Thread Functions
 * \ingroup l4util_api
 */

/**
 * \internal
 * \brief Create an L4 thread.
 * \ingroup l4util_thread
 * \note  You should only use this when you know what you're doing, thanks.
 * \param id          Cap-idx of new thread
 * \param thread_utcb Utcb of the new thread
 * \param factory     Factory to create the thread from
 * \param pc          Initial value of instruction pointer
 * \param sp          Initial value of stack pointer
 * \param pager       Pager of the thread
 * \param task        Task to put thread in
 * \param scheduler   Scheduler to use, specify L4_INVALID_CAP for not
 *                    calling the scheduler.
 * \param sp          Scheduler params to use
 * \return 0 on success, <0 on error
 */
L4_CV long
l4util_create_thread(l4_cap_idx_t id, l4_utcb_t *thread_utcb,
                     l4_cap_idx_t factory,
                     l4_umword_t pc, l4_umword_t sp, l4_cap_idx_t pager,
                     l4_cap_idx_t task,
                     l4_cap_idx_t scheduler, l4_sched_param_t scp) L4_NOTHROW;

L4_END_DECLS

#ifndef L4UTIL_THREAD_FUNC
/**
 * Defines a wrapper function that sets up the registers according
 * to the calling conventions for the architecture.
 *
 * Use this as a function header when starting a low-level thread
 * where only stack and instruction pointer are in a well-defined state.
 *
 * Example:
 *
 * L4UTIL_THREAD_FUNC(helper_thread)
 * {
 *   l4_infinite_loop();
 * }
 *
 * thread_cap->ex_regs((l4_umword_t)helper_thread, stack_addr);
 */
#define __L4UTIL_THREAD_FUNC(name) void L4_NORETURN name(void)
#define L4UTIL_THREAD_FUNC(name) __L4UTIL_THREAD_FUNC(name)
#define __L4UTIL_THREAD_STATIC_FUNC(name) static L4_NORETURN void name(void)
#define L4UTIL_THREAD_STATIC_FUNC(name) __L4UTIL_THREAD_STATIC_FUNC(name)
#endif

#endif /* __L4_THREAD_H */
