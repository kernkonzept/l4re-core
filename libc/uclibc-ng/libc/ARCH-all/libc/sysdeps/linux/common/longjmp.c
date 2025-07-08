/* Copyright (C) 1991, 92, 94, 95, 97, 98, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stddef.h>
#include <setjmp.h>
#include <signal.h>


extern void __longjmp (__jmp_buf __env, int __val) attribute_noreturn;
libc_hidden_proto(__longjmp)

extern __typeof(longjmp) __libc_longjmp attribute_noreturn;
/* Set the signal mask to the one specified in ENV, and jump
   to the position specified in ENV, causing the setjmp
   call there to return VAL, or 1 if VAL is 0.  */
void __libc_longjmp (sigjmp_buf env, int val)
{
#if 0
  /* Perform any cleanups needed by the frames being unwound.  */
  _longjmp_unwind (env, val);
#endif

#if 0
  /* FM3: signals not supported */
  if (env[0].__mask_was_saved)
    /* Restore the saved signal mask.  */
    (void) sigprocmask (SIG_SETMASK, &env[0].__saved_mask, NULL);
#endif

  /* Call the machine-dependent function to restore machine state.  */
  __longjmp (env[0].__jmpbuf, val ?: 1);
}

weak_alias(__libc_longjmp,longjmp)
weak_alias(__libc_longjmp,siglongjmp)
strong_alias(__libc_longjmp,__libc_siglongjmp)
strong_alias(__libc_longjmp,_longjmp)
