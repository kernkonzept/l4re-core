/* Define the machine-dependent type `jmp_buf'.  Nios2 version.
   Copyright (C) 1992,93,95,97,2000 Free Software Foundation, Inc.
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

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

typedef struct
{
    /* Callee-saved registers r16 through r23.  */
    unsigned long __regs[8];

    /* Program counter.  */
    unsigned long __pc;

    /* Stack pointer.  */
    unsigned long __sp;

    /* The frame pointer.  */
    unsigned long __fp;

    /* The global pointer.  */
    unsigned long __gp;

	/* floating point regs, if any */
#ifdef __UCLIBC_HAS_FPU__
    unsigned long __fpregs[64];
#endif
} __jmp_buf[1];

#endif	/* bits/setjmp.h */
