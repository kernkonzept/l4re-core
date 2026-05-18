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

#pragma once

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <l4/sys/scheduler.h>

#include <l4/util/arch/thread.h>

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
 * Defines a wrapper function that sets up the registers according to the
 * calling conventions for the architecture.
 *
 * Use this as a function header when starting a low-level thread where only
 * stack and instruction pointer are in a well-defined state.
 *
 * Example:
 *
 * \code
 * L4UTIL_THREAD_FUNC(helper_thread)
 * {
 *   l4_infinite_loop();
 * }
 *
 * thread_cap->ex_regs((l4_umword_t)helper_thread, stack_addr);
 * \endcode
 */
# define __L4UTIL_THREAD_FUNC(name) void L4_NORETURN name(void)
# define L4UTIL_THREAD_FUNC(name) __L4UTIL_THREAD_FUNC(name)
# define __L4UTIL_THREAD_STATIC_FUNC(name) static L4_NORETURN void name(void)
# define L4UTIL_THREAD_STATIC_FUNC(name) __L4UTIL_THREAD_STATIC_FUNC(name)
#endif

#ifndef L4UTIL_THREAD_CXX_FUNC_HELPER_PROTO_ATTR
# define L4UTIL_THREAD_CXX_FUNC_HELPER_PROTO_ATTR
#endif
#ifndef L4UTIL_THREAD_CXX_FUNC_INTERRUPT_HELPER_PROTO_ATTR
# define L4UTIL_THREAD_CXX_FUNC_INTERRUPT_HELPER_PROTO_ATTR
#endif

/**
 * Implement stub and handler.
 *
 * For 'Foo::foo(int a, int b)' this macro expands into:
 *
 * \code
 * asm (".global foo_stub    \n"
 *      "foo_stub:           \n"
 *      ... align stack pointer ...
 *      "<call> foo_from_asm \n");
 *
 * [[noreturn]] static void __attribute__((used))
 * Foo::foo(int a, int b) asm ("foo_from_asm")
 * \endcode
 */
#define _L4UTIL_THREAD_CXX_FUNC_IMPL_(suffix, class, fn_name, from_asm_name, ...)\
  asm (".global " L4_stringify(fn_name ## _stub)                  "\n"         \
       ".type " L4_stringify(fn_name ## _stub) " STT_FUNC          \n"         \
       L4_stringify(fn_name ## _stub) ":                           \n"         \
  L4UTIL_THREAD_CXX_FUNC_IMPL ## suffix ## _STUB(from_asm_name)                \
      );                                                                       \
                                                                               \
  [[noreturn]] void __attribute__((used)) class::from_asm_name(__VA_ARGS__)

#define _L4UTIL_THREAD_CXX_FUNC_IMPL(suffix, class, fn_name, ...)              \
  _L4UTIL_THREAD_CXX_FUNC_IMPL_(suffix, class, fn_name, fn_name ## _from_asm,  \
                                ##__VA_ARGS__)

#ifndef L4UTIL_THREAD_CXX_FUNC_PROTO

/**
 * Declare static C++ class method which can be called as handler were only
 * stack and instruction pointer are in a well-defined state.
 *
 * Architecture-specific implementations may override this macro to use certain
 * function attributes (e.g. __attribute__((regparm(3))) on x86).
 *
 * \note Use together with L4UTIL_THREAD_CXX_FUNC_IMPL() or
 *       L4UTIL_THREAD_CXX_FUNC_IMPL_SAME_STACK().
 */
# define L4UTIL_THREAD_CXX_FUNC_PROTO(fn_name, ...) \
   [[noreturn]] static void fn_name(__VA_ARGS__) \
   asm (L4_stringify(fn_name ## _stub)); \
   [[noreturn]] static void fn_name ## _from_asm(__VA_ARGS__) \
   asm (L4_stringify(fn_name ## _from_asm))

#endif

/**
 * Implement static C++ class method which can be called as handler were only
 * stack and instruction pointer are in a well-defined state.
 *
 * This is important for certain architectures defining global registers or for
 * architectures like AMD64 defining a "red zone" (a reserved fixed-size area
 * below the current stack pointer to be used for temporary data).
 *
 * The first two parameters of this macro define class name and method name,
 * followed by up to three function parameters.
 *
 * Use these macros like follows:
 *
 * \code
 * class Foo
 * {
 *   L4UTIL_THREAD_CXX_FUNC_PROTO(handler, int param1, long param2);
 * };
 *
 * L4UTIL_THREAD_CXX_FUNC_IMPL(Foo, handler, int param1, long param2)
 * {
 *   printf("param1 = %d, param2 = %ld\n", param1, param2);
 * }
 * \endcode
 *
 * to declare the method
 *
 * \code
 * [[noreturn]] void Foo::handler(int param1, long param2);
 * \endcode
 *
 * \note Use together with L4UTIL_THREAD_CXX_FUNC_PROTO().
 */

#ifdef L4UTIL_THREAD_CXX_FUNC_IMPL_STUB

# define L4UTIL_THREAD_CXX_FUNC_IMPL(class, fn_name, ...) \
   _L4UTIL_THREAD_CXX_FUNC_IMPL(, class, fn_name, ##__VA_ARGS__)

#else /* !L4UTIL_THREAD_CXX_FUNC_IMPL_STUB */

# define L4UTIL_THREAD_CXX_FUNC_IMPL(class, fn_name, ...) \
   [[noreturn]] void class::fn_name(__VA_ARGS__)

#endif

/**
 * Implement static C++ class method which can be called as handler were only
 * stack and instruction pointer are in a well-defined state.
 *
 * In contrast to L4UTIL_THREAD_CXX_FUNC_IMPL(), assume that this function may
 * interrupt code executed at another user-level context and there are adaptions
 * required to the stack pointer (alignment, consider the "red zone" on AMD64,
 * etc.)
 */
#ifndef L4UTIL_THREAD_CXX_FUNC_IMPL_INTERRUPT_STUB
# error architecture-specific L4UTIL_THREAD_CXX_FUNC_IMPL_INTERRUPT_STUB missing
#endif

#define L4UTIL_THREAD_CXX_FUNC_INTERRUPT_IMPL(class, fn_name, ...) \
  _L4UTIL_THREAD_CXX_FUNC_IMPL(_INTERRUPT, class, fn_name, ##__VA_ARGS__)
