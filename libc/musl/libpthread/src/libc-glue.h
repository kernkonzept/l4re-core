#pragma once

#include <features.h>
//#include <sys/cdefs.h>
//#include <bits/uClibc_page.h>

// TODO: Are all the following necessary?
#ifdef IS_IN_libpthread

#ifndef __GLIBC_HAVE_LONG_LONG
# define __GLIBC_HAVE_LONG_LONG
#endif

#define __getpagesize getpagesize
#define __sched_getscheduler sched_getscheduler
#define __sched_setscheduler sched_setscheduler
#define __sched_getparam sched_getparam
#define __getpid getpid
#define __gettimeofday gettimeofday
#define __poll poll
#define __sysctl sysctl
#define __open open
#define __read read
#define __close close
#define __on_exit on_exit
#define __libc_current_sigrtmin_private __libc_current_sigrtmin
#define __clone clone

extern void *__libc_stack_end;
extern int __cxa_atexit (void (*func) (void *), void *arg, void *d);

#endif /* IS_IN_libpthread */

// TODO: What to do in musl??
#ifdef __UCLIBC_HAS_XLOCALE__
# define __uselocale(x) uselocale(x)
#else
# define __uselocale(x) ((void)0)
#endif

/* Type for array elements in 'cpu_set_t'.  */
typedef unsigned long int __cpu_mask;

/* Use a funky version in a probably vein attempt at preventing gdb 
 * from dlopen()'ing glibc's libthread_db library... */
#define STRINGIFY(s) STRINGIFY2 (s)
#define STRINGIFY2(s) #s
#define VERSION STRINGIFY(__UCLIBC_MAJOR__) "." STRINGIFY(__UCLIBC_MINOR__) "." STRINGIFY(__UCLIBC_SUBLEVEL__)

#define attribute_hidden __attribute__ ((visibility ("hidden")))
#define attribute_protected __attribute__ ((visibility ("protected")))
#define attribute_noreturn __attribute__ ((__noreturn__))
#define attribute_unused __attribute__ ((unused))

#define internal_function  /* empty */

/* Define ALIASNAME as a strong alias for NAME.  */
#ifndef strong_alias
# define strong_alias(name, aliasname) _strong_alias(name, aliasname)
# define _strong_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name))) __attribute_copy__ (name);
#endif

#ifndef weak_alias
#  define weak_alias(name, aliasname) _weak_alias (name, aliasname)
#  define _weak_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((weak, alias (#name))) __attribute_copy__ (name);
#endif
# define __attribute_copy__(arg) __attribute__ ((__copy__ (arg)))

#ifndef __set_errno
#define __set_errno(val) (errno = (val))
#endif

# define __MAX_ALLOCA_CUTOFF  65536

// All our arches grow down
#define _STACK_GROWS_DOWN 1

/* Forces a function to be always inlined.  */
# define __always_inline __inline __attribute__ ((__always_inline__))

# if defined __GNUC_STDC_INLINE__ || defined __GNUC_GNU_INLINE__ || defined __cplusplus
#  define __extern_inline extern __inline __attribute__ ((__gnu_inline__))
#  define __extern_always_inline \
     extern __always_inline __attribute__ ((__gnu_inline__, __artificial__))
# else
#  define __extern_inline extern __inline
#  define __extern_always_inline extern __always_inline
# endif
