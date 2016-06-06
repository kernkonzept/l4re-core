/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Define the machine-dependent type `jmp_buf'.  NDS32 version. */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H  1

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifndef _ASM
typedef struct
  {
    /* Callee-saved registers r6 - r14, r16 - r19 and r28 - r31.  */
    int __regs[31];

    /* Program counter.  */
    void * __pc;
  } __jmp_buf[1];
#endif

#endif  /* bits/setjmp.h */
