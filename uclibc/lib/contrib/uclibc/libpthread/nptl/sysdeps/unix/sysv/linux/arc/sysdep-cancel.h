/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <tls.h>
#include <sysdep.h>
#ifndef __ASSEMBLER__
# include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

#ifdef __ASSEMBLER__

#undef ret
#define ret

# undef PSEUDO
# define PSEUDO(name, syscall_name, nargs)			\
   /* vanilla version */				`	\
   ENTRY(name##_nocancel)				`	\
      DO_CALL (__NR_##syscall_name)			`	\
      jls  [blink]					`	\
      b  __syscall_error@plt				`	\
   END(name##_nocancel)					`	\
   /* thread cancellation variant */			`	\
   ENTRY(name)			    			`	\
      SINGLE_THREAD_P					`	\
      bz name##_nocancel				`	\
      DOCARGS_##nargs  /* stash syscall args */		`	\
      CENABLE          /* call enable_asynccancel */	`	\
      mov r9, r0       /* Safe-keep mask */ 		`	\
      UNDOCARGS_##nargs	/* restore syscall args */	`	\
      DO_CALL (__NR_##syscall_name)			`	\
      push  r0         /* save syscall return value */	`	\
      mov   r0, r9     /* prep mask for disable_asynccancel */  `	\
      CDISABLE	`	\
      pop  r0           /* get syscall ret value back */  ` \
      pop  blink	/* UNDOCARGS above left blink on stack */ `	\
      cmp  r0, -1024	`	\
      jls  [blink]					`	\
      b  __syscall_error@plt				`	\
   END(name)

#undef	PSEUDO_END
#define	PSEUDO_END(name)	\

# ifdef IS_IN_libpthread
#  define CENABLE	bl __pthread_enable_asynccancel
#  define CDISABLE	bl __pthread_disable_asynccancel
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define CENABLE	bl __libc_enable_asynccancel
#  define CDISABLE	bl __libc_disable_asynccancel
#  define __local_multiple_threads __libc_multiple_threads
# elif defined IS_IN_librt
#  define CENABLE	bl __librt_enable_asynccancel
#  define CDISABLE	bl __librt_disable_asynccancel
# else
#  error Unsupported library
# endif

#define DO_CALL(num)		\
	mov	r8, num		`	\
	ARC_TRAP_INSN		`	\
	cmp r0, -1024

.macro push reg
	st.a \reg, [sp, -4]
.endm

.macro pop reg
	ld.ab \reg, [sp, 4]
.endm

#define DOCARGS_0	push blink

/* don't pop blink at this point */
#define UNDOCARGS_0	ld   blink, [sp]

#define DOCARGS_1	DOCARGS_0`	push r0
#define UNDOCARGS_1			pop  r0`	UNDOCARGS_0

#define DOCARGS_2	DOCARGS_1`	push r1
#define UNDOCARGS_2			pop  r1`	UNDOCARGS_1

#define DOCARGS_3	DOCARGS_2`	push r2
#define UNDOCARGS_3			pop  r2`	UNDOCARGS_2

#define DOCARGS_4	DOCARGS_3`	push r3
#define UNDOCARGS_4			pop  r3`	UNDOCARGS_3

#define DOCARGS_5	DOCARGS_4`	push r4
#define UNDOCARGS_5			pop  r4`	UNDOCARGS_4

#define DOCARGS_6	DOCARGS_5`	push r5
#define UNDOCARGS_6			pop  r5`	UNDOCARGS_5

#define DOCARGS_7	DOCARGS_6`	push r6
#define UNDOCARGS_7			pop  r6`	UNDOCARGS_6

#  define SINGLE_THREAD_P 			\
    THREAD_SELF r9   `				\
    ld	   r10, [r9, MULTIPLE_THREADS_OFFSET]`	\
    cmp    r10, 0

/*    ld	   r2, [r1, -TLS_PRE_TCB_SIZE + MULTIPLE_THREADS_OFFSET] */
#else	/* !__ASSEMBLER__ */

/* TBD: Can use @__local_multiple_threads for libc/libpthread like ARM */
#   define SINGLE_THREAD_P	\
    likely(THREAD_GETMEM (THREAD_SELF, header.multiple_threads) == 0)

#endif /* __ASSEMBLER__ */

#endif
