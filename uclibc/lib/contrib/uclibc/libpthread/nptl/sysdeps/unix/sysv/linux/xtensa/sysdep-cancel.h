/* Copyright (C) 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <tls.h>
/* #include <pt-machine.h> */
#ifndef __ASSEMBLER__
# include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

#ifdef __ASSEMBLER__
#if defined(__XTENSA_WINDOWED_ABI__)
/* CENABLE/CDISABLE in PSEUDO below use call8, stack frame size must be
 * at least 32.
 */
#if FRAMESIZE < 32
#undef FRAMESIZE
#define FRAMESIZE 32
#endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    SINGLE_THREAD_P(a15);						      \
    bnez     a15, .Lpseudo_cancel; 					      \
    DO_CALL (syscall_name, args);					      \
    bgez     a2, .Lpseudo_done; 					      \
    movi     a4, -4095;							      \
    blt      a2, a4, .Lpseudo_done; 					      \
    j        SYSCALL_ERROR_LABEL;					      \
  .Lpseudo_done: 							      \
    retw;								      \
  .Lpseudo_cancel: 							      \
    /* The syscall args are in a2...a7; no need to save */		      \
    CENABLE;								      \
    /* The return value is in a10 and preserved across the syscall */	      \
    DO_CALL (syscall_name, args);					      \
    CDISABLE;								      \
    bgez     a2, .Lpseudo_end;                                                \
    movi     a4, -4095;							      \
    blt      a2, a4, .Lpseudo_end;                                            \
    j        SYSCALL_ERROR_LABEL;					      \
  .Lpseudo_end:

# define CENABLE	movi    a8, CENABLE_FUNC;		\
			callx8  a8
# define CDISABLE	movi    a8, CDISABLE_FUNC;		\
			callx8  a8
#elif defined(__XTENSA_CALL0_ABI__)

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    SINGLE_THREAD_P(a10);						      \
    bnez     a10, .Lpseudo_cancel;					      \
    DO_CALL (syscall_name, args);					      \
    bgez     a2, .Lpseudo_done;						      \
    movi     a4, -4095;							      \
    blt      a2, a4, .Lpseudo_done;					      \
    j        SYSCALL_ERROR_LABEL;					      \
  .Lpseudo_done:							      \
    ret;								      \
  .Lpseudo_cancel:							      \
    addi     a1, a1, -32;						      \
    /* The syscall args are in a2...a7; save them */			      \
    s32i     a0, a1, 0;							      \
    s32i     a2, a1, 4;							      \
    s32i     a3, a1, 8;							      \
    s32i     a4, a1, 12;						      \
    s32i     a5, a1, 16;						      \
    s32i     a6, a1, 20;						      \
    s32i     a7, a1, 24;						      \
    CENABLE;								      \
    /* Move return value to a10 preserved across the syscall */		      \
    mov      a10, a2;							      \
    l32i     a2, a1, 4;							      \
    l32i     a3, a1, 8;							      \
    l32i     a4, a1, 12;						      \
    l32i     a5, a1, 16;						      \
    l32i     a6, a1, 20;						      \
    l32i     a7, a1, 24;						      \
    DO_CALL (syscall_name, args);					      \
    s32i     a2, a1, 4;							      \
    mov      a2, a10;							      \
    CDISABLE;								      \
    l32i     a2, a1, 4;							      \
    l32i     a0, a1, 0;							      \
    addi     a1, a1, 32;						      \
    bgez     a2, .Lpseudo_end;                                                \
    movi     a4, -4095;							      \
    blt      a2, a4, .Lpseudo_end;                                            \
    j        SYSCALL_ERROR_LABEL;					      \
  .Lpseudo_end:

# define CENABLE	movi    a0, CENABLE_FUNC;		\
			callx0  a0
# define CDISABLE	movi    a0, CDISABLE_FUNC;		\
			callx0  a0
#else
#error Unsupported Xtensa ABI
#endif
#endif

# ifdef IS_IN_libpthread
#  define CENABLE_FUNC	__pthread_enable_asynccancel
#  define CDISABLE_FUNC	__pthread_disable_asynccancel
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define CENABLE_FUNC	__libc_enable_asynccancel
#  define CDISABLE_FUNC	__libc_disable_asynccancel
#  define __local_multiple_threads __libc_multiple_threads
# elif defined IS_IN_librt
#  define CENABLE_FUNC	__librt_enable_asynccancel
#  define CDISABLE_FUNC	__librt_disable_asynccancel
# else
#  error Unsupported library
# endif

# if defined IS_IN_libpthread || !defined NOT_IN_libc
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P(reg) movi reg, __local_multiple_threads; \
			        l32i reg, reg, 0;
#  endif

# else
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P \
	__builtin_expect (THREAD_GETMEM (THREAD_SELF, \
			  header.multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P(reg) \
	rur reg, threadptr; \
	l32i reg, reg, MULTIPLE_THREADS_OFFSET;
#  endif
# endif

#else

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION 1

#endif


#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
#endif
