/* setjmp for or1k

   Based on:
   setjmp for PowerPC.
   Copyright (C) 1995, 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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

#include "spr_defs.h"
#include <jmpbuf-offsets.h>

#ifdef __UCLIBC_HAS_FLOATS__
#define FP(x...) x
#else
#define FP(x...)
#endif

.globl _setjmp
.type _setjmp, @function
.align 2

_setjmp:
	l.addi	r4,r0,0			/* Set second argument to 0.  */
	l.j	.Local_sigsetjmp
	l.nop
.size     _setjmp,.-_setjmp

.globl __setjmp
.type __setjmp, @function
.align 2

__setjmp:
	l.addi	r4,r0,1			/* Set second argument to 1.  */
	l.j	.Local_sigsetjmp
	l.nop
.size __setjmp,.-__setjmp

.globl setjmp
.set setjmp,__setjmp

.globl __sigsetjmp
.type __sigsetjmp, @function
.align 2

__sigsetjmp:
.Local_sigsetjmp:
#	l.mfspr	r11,r0,SPR_SR
#	l.sw    (JB_SR*4)(r3),r11
	l.sw    ((JB_GPRS+0)*4)(r3),r1
	l.sw    ((JB_GPRS+1)*4)(r3),r2
/*	l.sw    ((JB_GPRS+2)*4)(r3),r3*/
	l.sw    ((JB_GPRS+3)*4)(r3),r4
	l.sw    ((JB_GPRS+4)*4)(r3),r5
	l.sw    ((JB_GPRS+5)*4)(r3),r6
	l.sw    ((JB_GPRS+6)*4)(r3),r7
	l.sw    ((JB_GPRS+7)*4)(r3),r8
	l.sw    ((JB_GPRS+8)*4)(r3),r9
	l.sw    ((JB_GPRS+9)*4)(r3),r10
	l.sw    ((JB_GPRS+10)*4)(r3),r11
	l.sw    ((JB_GPRS+11)*4)(r3),r12
	l.sw    ((JB_GPRS+12)*4)(r3),r13
	l.sw    ((JB_GPRS+13)*4)(r3),r14
	l.sw    ((JB_GPRS+14)*4)(r3),r15
	l.sw    ((JB_GPRS+15)*4)(r3),r16
	l.sw    ((JB_GPRS+16)*4)(r3),r17
	l.sw    ((JB_GPRS+17)*4)(r3),r18
	l.sw    ((JB_GPRS+18)*4)(r3),r19
	l.sw    ((JB_GPRS+19)*4)(r3),r20
	l.sw    ((JB_GPRS+20)*4)(r3),r21
	l.sw    ((JB_GPRS+21)*4)(r3),r22
	l.sw    ((JB_GPRS+22)*4)(r3),r23
	l.sw    ((JB_GPRS+23)*4)(r3),r24
	l.sw    ((JB_GPRS+24)*4)(r3),r25
	l.sw    ((JB_GPRS+25)*4)(r3),r26
	l.sw    ((JB_GPRS+26)*4)(r3),r27
	l.sw    ((JB_GPRS+27)*4)(r3),r28
	l.sw    ((JB_GPRS+28)*4)(r3),r29
	l.sw    ((JB_GPRS+29)*4)(r3),r30
	l.sw    ((JB_GPRS+30)*4)(r3),r31
	l.j	__sigjmp_save
	l.nop
.size __sigsetjmp,.-__sigsetjmp
