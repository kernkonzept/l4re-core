/* Copyright (C) 2003, 2004, 2005, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <tls.h>
#include <sysdep.h>
#ifndef __ASSEMBLER__
# include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

/* NOTE: We do mark syscalls with unwind annotations, for the benefit of
   cancellation; but they're really only accurate at the point of the
   syscall.  The ARM unwind directives are not rich enough without adding
   a custom personality function.  */

#ifdef __ASSEMBLER__
#undef ret
#define ret								\
   CMP D0Re0, #-4095;							\
   MOVLO PC, D1RtP;							\
   MOV D1Ar1, D0Re0;							\
   B SYSCALL_ERROR;
#endif /* __ASSEMBLER__ */

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
  .type ___##syscall_name##_nocancel,%function;				\
  .globl ___##syscall_name##_nocancel;					\
  ___##syscall_name##_nocancel:						\
    cfi_startproc;							\
    DO_CALL (syscall_name, args);					\
    MOV PC, D1RtP;							\
    cfi_endproc;							\
  .size ___##syscall_name##_nocancel,.-___##syscall_name##_nocancel;	\
    .globl _##name;							\
    .type _##name, @function;						\
name##:									\
_##name##:								\
    DOCARGS_##args;							\
    SINGLE_THREAD_P;							\
    UNDOCARGS_##args;							\
    BNE .Lpseudo_cancel;						\
    cfi_remember_state;							\
    DO_CALL (syscall_name, 0);						\
    ret									\
    cfi_restore_state;							\
  .Lpseudo_cancel:							\
    MSETL [A0StP++], D0FrT, D0.5;					\
    DOCARGS_##args;	/* save syscall args etc. around CENABLE.  */	\
    CENABLE;								\
    MOV D0FrT, D0Re0;	/* put mask in safe place.  */			\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    DO_CALL(syscall_name, 0);	/* do the call.  */			\
    MOV D0.5, D0Re0;	/* save syscall return value.  */		\
    MOV D1Ar1, D0FrT;	/* get mask back.  */				\
    CDISABLE;								\
    MOV D0Re0, D0.5;	/* retrieve return value.  */			\
    GETL D0.5, D1.5, [--A0StP];						\
    GETL D0FrT, D1RtP, [--A0StP];

# define DOCARGS_0
# define UNDOCARGS_0

# define DOCARGS_1 \
  SETL [A0StP++], D1Ar1, D0Ar2
# define UNDOCARGS_1 \
  GETL D1Ar1, D0Ar2, [--A0StP]

# define DOCARGS_2 DOCARGS_1

# define UNDOCARGS_2 UNDOCARGS_2

# define DOCARGS_3 \
  MSETL [A0StP++], D1Ar1, D1Ar3

# define UNDOCARGS_3 \
  GETL D1Ar1, D0Ar2, [--A0StP];		\
  GETL D1Ar3, D0Ar4, [--A0StP]

# define DOCARGS_4 DOCARGS_3
# define UNDOCARGS_4 UNDOCARGS_3

# define DOCARGS_5 \
  MSETL [A0StP++], D1Ar1, D1Ar3, D1Ar5
# define UNDOCARGS_5 \
  GETL D1Ar1, D0Ar2, [--A0StP];		\
  GETL D1Ar3, D0Ar4, [--A0StP];		\
  GETL D1Ar5, D0Ar6, [--A0StP]

# define DOCARGS_6 DOCARGS_5
# define UNDOCARGS_6 UNDOCARGS_5

# ifdef IS_IN_libpthread
#  define CENABLE	CALLR D1RtP, ___pthread_enable_asynccancel@PLT
#  define CDISABLE	CALLR D1RtP, ___pthread_disable_asynccancel@PLT
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define CENABLE	CALLR D1RtP, ___libc_enable_asynccancel@PLT
#  define CDISABLE	CALLR D1RtP, ___libc_disable_asynccancel@PLT
#  define __local_multiple_threads __libc_multiple_threads
# elif defined IS_IN_librt
#  define CENABLE	CALLR D1RtP, ___librt_enable_asynccancel@PLT
#  define CDISABLE	CALLR D1RtP, ___librt_disable_asynccancel@PLT
# else
#  error Unsupported library
# endif

#ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P						\
    likely(THREAD_GETMEM (THREAD_SELF,			\
				   header.multiple_threads) == 0)
#else
#  define SINGLE_THREAD_P \
	SETL	[A0StP++], D0FrT, D1RtP; \
	CALLR	D1RtP, ___metag_load_tp@PLT; \
	SUB	D0Re0, D0Re0, #TLS_PRE_TCB_SIZE;	\
	GETD	D0Re0, [D0Re0 + #MULTIPLE_THREADS_OFFSET]; \
	CMP	D0Re0, #0; \
	GETL	D0FrT, D1RtP, [--A0StP]
#endif


#elif !defined __ASSEMBLER__

/* For rtld, et cetera.  */
# define SINGLE_THREAD_P 1
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
    likely(THREAD_GETMEM (THREAD_SELF,  \
				   header.multiple_threads) == 0)
#endif
