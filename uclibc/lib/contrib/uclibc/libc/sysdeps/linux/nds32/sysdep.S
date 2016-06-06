/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 1991-2003 Free Software Foundation, Inc.

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
#define _ERRNO_H
#include <bits/errno.h>

.text

.globl C_SYMBOL_NAME(errno)
.globl __syscall_error

ENTRY (__syscall_error)
#ifdef OLD_ABI
	subri $r5, $r5, #0
#else
	subri $r0, $r0, #0
#endif

#define __syscall_error __syscall_error_1

#undef syscall_error
#ifdef NO_UNDERSCORES
__syscall_error:
#else
syscall_error:
#endif

#ifdef PIC
	/* set GP register */
	pushm	$gp, $lp
#ifdef __NDS32_N1213_43U1H__
	jal	2f
	sethi	$gp,	hi20(_GLOBAL_OFFSET_TABLE_)
	ori	$gp,	$gp,	lo12(_GLOBAL_OFFSET_TABLE_+4)
	add	$gp,	$gp,	$lp
#else
	mfusr	$r15, $PC
	sethi	$gp,	hi20(_GLOBAL_OFFSET_TABLE_+4)
	ori	$gp,	$gp,	lo12(_GLOBAL_OFFSET_TABLE_+8)
	add	$gp,	$gp,	$r15
#endif
#endif

#if defined (EWOULDBLOCK_sys) && EWOULDBLOCK_sys != EAGAIN
	/* We translate the system's EWOULDBLOCK error into EAGAIN.
	   The GNU C library always defines EWOULDBLOCK==EAGAIN.
	   EWOULDBLOCK_sys is the original number.  */
	   push	$t0
	   li	$t0, EWOULDBLOCK_sys
	   bne	$r0, $t0, 1f
	   pop	$t0
	   li	$r0, EAGAIN
1:
#endif

#ifdef _LIBC_REENTRANT
	push	$lp
	push	$r0
#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP)
#else
	addi	$sp,	$sp, -24
#endif

#ifdef PIC
	bal	C_SYMBOL_NAME(__errno_location@PLT)
#else
	bal	C_SYMBOL_NAME(__errno_location)
#endif
#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP)
#else
	addi	$sp,	$sp, 24
#endif
	pop	$r1

	swi	$r1, [$r0]
	li	$r0, -1
	pop	$lp
#ifdef PIC
	/* restore GP register */
	popm	$gp, $lp
#endif
2:
	ret
#else
#ifndef	PIC
	l.w	$r1, .L1
	swi	$r0, [$r1]
	li	$r0, -1
	ret

.L1:	.long C_SYMBOL_NAME(errno)
#else
	s.w	$r0, errno@GOTOFF
	li	$r0, -1
	
	/* restore GP register */
	popm	$gp, $lp
2:
	ret

#endif
#endif

#undef __syscall_error
END (__syscall_error)
