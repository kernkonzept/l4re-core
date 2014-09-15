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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <tls.h>
/* #include <pt-machine.h> */
#ifndef __ASSEMBLER__
# include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt
// FIXME: ENTRY includes an entry instruction, here we'd want entry sp, 48!
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

# define CENABLE	movi    a8, CENABLE_FUNC;		\
			callx8  a8
# define CDISABLE	movi    a8, CDISABLE_FUNC;		\
			callx8  a8

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
