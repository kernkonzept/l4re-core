/* longjmp for or1k

   Based on:
   longjmp for PowerPC.
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

#include <jmpbuf-offsets.h>

#ifdef __UCLIBC_HAS_FLOATS__
#define FP(x...) x
#else
#define FP(x...)
#endif

.globl     __longjmp;
.type      __longjmp,@function;
.align     2;

__longjmp:    
#	l.lwz   r11,(JB_SR*4)(r3)
#	l.mtspr	r0,r11,SPR_SR
	l.lwz   r1,((JB_GPRS+0)*4)(r3)
	l.lwz   r2,((JB_GPRS+1)*4)(r3)
	/* pass through "value" to r11, then restore r4, for what it's worth" */
#if 1	
	/* if r4 is 0, something wrong, so set it to 1 */
	l.sfeqi r4, 0x0
	l.bnf   1f /* r4 != 0, longjmp value sensible */
	l.nop
	l.ori   r4, r0, 0x1 /* make nonzero */
1:
#endif	
	l.addi	r11, r4, 0
	l.lwz   r4,((JB_GPRS+3)*4)(r3)
	l.lwz   r5,((JB_GPRS+4)*4)(r3)
	l.lwz   r6,((JB_GPRS+5)*4)(r3)
	l.lwz   r7,((JB_GPRS+6)*4)(r3)
	l.lwz   r8,((JB_GPRS+7)*4)(r3)
	l.lwz   r9,((JB_GPRS+8)*4)(r3)
	l.lwz   r10,((JB_GPRS+9)*4)(r3)
	l.lwz   r12,((JB_GPRS+11)*4)(r3)
	l.lwz   r13,((JB_GPRS+12)*4)(r3)
	l.lwz   r14,((JB_GPRS+13)*4)(r3)
	l.lwz   r15,((JB_GPRS+14)*4)(r3)
	l.lwz   r16,((JB_GPRS+15)*4)(r3)
	l.lwz   r17,((JB_GPRS+16)*4)(r3)
	l.lwz   r18,((JB_GPRS+17)*4)(r3)
	l.lwz   r19,((JB_GPRS+18)*4)(r3)
	l.lwz   r20,((JB_GPRS+19)*4)(r3)
	l.lwz   r21,((JB_GPRS+20)*4)(r3)
	l.lwz   r22,((JB_GPRS+21)*4)(r3)
	l.lwz   r23,((JB_GPRS+22)*4)(r3)
	l.lwz   r24,((JB_GPRS+23)*4)(r3)
	l.lwz   r25,((JB_GPRS+24)*4)(r3)
	l.lwz   r26,((JB_GPRS+25)*4)(r3)
	l.lwz   r27,((JB_GPRS+26)*4)(r3)
	l.lwz   r28,((JB_GPRS+27)*4)(r3)
	l.lwz   r29,((JB_GPRS+28)*4)(r3)
	l.lwz   r30,((JB_GPRS+29)*4)(r3)
	l.lwz   r31,((JB_GPRS+30)*4)(r3)
	l.jr	r9
	l.nop
.size     __longjmp,.-__longjmp

libc_hidden_def(__longjmp)
