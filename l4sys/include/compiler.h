/*****************************************************************************/
/**
 * \file
 * L4 compiler related defines.
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Jork Löser <jork@os.inf.tu-dresden.de>,
 *               Ronald Aigner <ra3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/*****************************************************************************/
#ifndef __L4_COMPILER_H__
#define __L4_COMPILER_H__

#if !defined(__ASSEMBLY__) && !defined(__ASSEMBLER__)

/**
 * \addtogroup l4sys_defines
 *
 * \includefile{l4/sys/compiler.h}
 */
/**@{*/

/**
 * %L4 Inline function attribute.
 * \hideinitializer
 */
#ifndef L4_INLINE
#ifndef __cplusplus
#  ifdef __OPTIMIZE__
#    define L4_INLINE_STATIC static __inline__
#    define L4_INLINE_EXTERN extern __inline__
#    ifdef __GNUC_STDC_INLINE__
#      define L4_INLINE L4_INLINE_STATIC
#    else
#      define L4_INLINE L4_INLINE_EXTERN
#    endif
#  else /* ! __OPTIMIZE__ */
#    define L4_INLINE static
#  endif /* ! __OPTIMIZE__ */
#else /* __cplusplus */
#  define L4_INLINE inline
#endif  /* __cplusplus */
#elif defined DOXYGEN
#  define L4_INLINE inline
#endif  /* L4_INLINE */

/**
 * Always inline a function
 * \hideinitializer
 */
#define L4_ALWAYS_INLINE L4_INLINE __attribute__((__always_inline__))


#define L4_DECLARE_CONSTRUCTOR(func, prio) \
  static inline __attribute__((constructor(prio))) void func ## _ctor_func(void) { func(); }


/**
 * Start section with C types and functions.
 * \def     L4_BEGIN_DECLS
 * \hideinitializer
 */
/**
 * End section with C types and functions.
 * \def     L4_END_DECLS
 * \hideinitializer
 */

/**
 * \def L4_NOTHROW
 * \hideinitializer
 * Mark a function declaration and definition as never throwing an exception.
 * (Also for C code).
 *
 * This macro shall be used to mark C and C++ functions that never
 * throw any exception.  Note that also C functions may throw exceptions
 * according to the compilers ABI and shall be marked with L4_NOTHROW
 * if they never do.  In C++ this is equivalent to \c throw().
 *
 * \code
 * int foo() L4_NOTHROW;
 * ...
 * int foo() L4_NOTHROW
 * {
 *   ...
 *   return result;
 * }
 * \endcode
 *
 */

/**
 * \def L4_EXPORT
 * \hideinitializer
 * Attribute to mark functions, variables, and data types as being exported
 * from a library.
 *
 * All data types, functions, and global variables that shall be exported
 * from a library shall be marked with this attribute.  The default may become
 * to hide everything that is not marked as L4_EXPORT from the users of a
 * library and provide the possibility for aggressive optimization of all
 * those internal functionality of a library.
 *
 * Usage:
 * \code
 * class L4_EXPORT My_class
 * {
 *   ...
 * };
 *
 * int L4_EXPORT function(void);
 *
 * int L4_EXPORT global_data; // global data is not recommended
 * \endcode
 *
 */

/**
 * \def L4_HIDDEN
 * \hideinitializer
 * Attribute to mark functions, variables, and data types as being explicitly
 * hidden from users of a library.
 *
 * This attribute is intended for functions, data, and data types that
 * shall never be visible outside of a library.  In particular, for shared
 * libraries this may result in much faster code within the library and short
 * linking times.
 *
 * \code
 * class L4_HIDDEN My_class
 * {
 *   ...
 * };
 *
 * int L4_HIDDEN function(void);
 *
 * int L4_HIDDEN global_data; // global data is not recommended
 * \endcode
 */
#ifndef __cplusplus
#  define L4_NOTHROW__A       __attribute__((nothrow))
#  define L4_NOTHROW
#  ifndef __BEGIN_DECLS
#    define __BEGIN_DECLS
#  endif
#  ifndef __END_DECLS
#    define __END_DECLS
#  endif
#  define L4_BEGIN_DECLS
#  define L4_END_DECLS
#  define L4_DEFAULT_PARAM(x)
#else /* __cplusplus */
#  if __cplusplus >= 201103L
#    define L4_NOTHROW noexcept
#  else /* C++ < 11 */
#    define L4_NOTHROW throw()
#  endif
#  define L4_BEGIN_DECLS extern "C" {
#  define L4_END_DECLS }
#  if !defined __BEGIN_DECLS || defined DOXYGEN
#    define __BEGIN_DECLS extern "C" {
#  endif
#  if !defined __END_DECLS || defined DOXYGEN
#    define __END_DECLS }
#  endif
#  define L4_DEFAULT_PARAM(x) = x
#endif /* __cplusplus */

/* Depration hints during compile -- remove later (2025+) */
#ifndef EXTERN_C
#define EXTERN_C DO_NOT_USE_EXTERN_C_ANY_MORE
#endif
#ifndef EXTERN_C_BEGIN
#define EXTERN_C_BEGIN DO_NOT_USE_EXTERN_C_BEGIN_ANY_MORE__USE_L4_BEGIN_DECLS
#endif
#ifndef EXTERN_C_END
#define EXTERN_C_END DO_NOT_USE_EXTERN_C_END_ANY_MORE__USE_L4_END_DECLS
#endif

/**
 * Constexpr function attribute.
 * \hideinitializer
 */
#if defined __cplusplus && __cplusplus >= 201402L
# define L4_CONSTEXPR constexpr
#else
# define L4_CONSTEXPR
#endif

/**
 * Noreturn function attribute.
 * \hideinitializer
 */
#define L4_NORETURN __attribute__((noreturn))

#define L4_PURE __attribute__((pure))

/**
 * No instrumentation function attribute.
 * \hideinitializer
 */
#define L4_NOINSTRUMENT __attribute__((no_instrument_function))
#ifndef L4_HIDDEN
#  define L4_HIDDEN __attribute__((visibility("hidden")))
#endif
#if !defined L4_EXPORT || defined DOXYGEN
#  define L4_EXPORT __attribute__((visibility("default")))
#endif
#ifndef L4_EXPORT_TYPE
#  ifdef __cplusplus
#    define L4_EXPORT_TYPE __attribute__((visibility("default")))
#  else
#    define L4_EXPORT_TYPE
#  endif
#endif
#define L4_STRONG_ALIAS(name, aliasname) L4__STRONG_ALIAS(name, aliasname)
#ifdef __clang__
#define L4__STRONG_ALIAS(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name)));
#else
#define L4__STRONG_ALIAS(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name), copy(name)));
#endif

/**
 * Specify the desired alignment of the stack pointer.
 *
 * __BIGGEST_ALIGNMENT__ provides the largest alignment ever used for any data
 * type on the target machine. This is normally identical to desired stack
 * alignment.
 */
#if defined(__i386__) || defined(__amd64__) || \
  defined(__arm__) || defined(__aarch64__) || \
  defined(__mips__) || defined(__riscv) || \
  defined(__powerpc__) || defined(__sparc__)
# define L4_STACK_ALIGN         __BIGGEST_ALIGNMENT__
#else
# error Define L4_STACK_ALIGN for this target!
#endif

/**
 * Align stack pointer for directly invoked functions.
 *
 * The stack needs to be aligned to L4_STACK_ALIGN for being able to access
 * certain data on the stack. On x86/AMD64, a function call is performed using
 * the 'call' instruction decrementing the stack pointer and writing the return
 * address onto the stack. The called function considers this when adapting the
 * stack pointer after function entry. If the called function was not invoked
 * by a 'call' instruction, the stack pointer is actually off by a machine word
 * leading to stack alignment issues when executing SSE instructions.
 *
 * This function fixes the stack pointer for directly invoked functions. For
 * architectures not automatically pushing the stack pointer during a function
 * call, just enforce the L4_STACK_ALIGN alignment.
 *
 * \hideinitializer
 */
#if defined(__i386__) || defined(__amd64__)
L4_INLINE unsigned long l4_align_stack_for_direct_fncall(unsigned long stack)
{
  if ((stack & (L4_STACK_ALIGN - 1)) == (L4_STACK_ALIGN - sizeof(unsigned long)))
    return stack;
  return (stack & ~(L4_STACK_ALIGN)) - sizeof(unsigned long);
}
#else
L4_INLINE unsigned long l4_align_stack_for_direct_fncall(unsigned long stack)
{
  return stack & ~(L4_STACK_ALIGN);
}
#endif

#endif /* !__ASSEMBLY__ */

#include <l4/sys/linkage.h>

#define L4_LIKELY(x)	__builtin_expect((x),1)   ///< Expression is likely to execute. \hideinitializer
#define L4_UNLIKELY(x)	__builtin_expect((x),0)   ///< Expression is unlikely to execute. \hideinitializer

/* Make sure that the function is not removed by optimization. Without the
 * "used" attribute, unreferenced static functions are removed. */
#define L4_STICKY(x)     __attribute__((used)) x         ///< Mark symbol sticky (even not there) \hideinitializer
#define L4_DEPRECATED(s) __attribute__((deprecated(s)))  ///< Mark symbol deprecated. \hideinitializer

#ifndef static_assert
# if !defined(__cplusplus)
#  define static_assert(x, y) _Static_assert(x, y)
# elif __cplusplus < 201103L
#  define static_assert(x, y) \
   extern int l4_static_assert[-(!(x))] __attribute__((unused))
# endif
#endif

#define L4_stringify_helper(x) #x                       ///< stringify helper. \hideinitializer
#define L4_stringify(x)        L4_stringify_helper(x)   ///< stringify. \hideinitializer

#ifdef __has_builtin
#define L4_HAS_BUILTIN(def) __has_builtin(def)
#else
#define L4_HAS_BUILTIN(def) 0
#endif

#ifndef __ASSEMBLER__
/**
 * Memory barrier.
 */
L4_INLINE void l4_barrier(void);

/**
 * Memory barrier.
 */
L4_INLINE void l4_mb(void);

/**
 * Write memory barrier.
 */
L4_INLINE void l4_wmb(void);

/**
 * Infinite loop. Will never return. Use #l4_sleep_forever() if at all possible.
 */
L4_INLINE L4_NORETURN void l4_infinite_loop(void);


/* Implementations */
L4_INLINE void l4_barrier(void)
{
  __asm__ __volatile__ ("" : : : "memory");
}

L4_INLINE void l4_mb(void)
{
  __asm__ __volatile__ ("" : : : "memory");
}

L4_INLINE void l4_wmb(void)
{
  __asm__ __volatile__ ("" : : : "memory");
}

L4_INLINE L4_NORETURN void l4_infinite_loop(void)
{
  while (1)
    l4_barrier();
}
#endif

/**@}*/

#endif /* !__L4_COMPILER_H__ */
