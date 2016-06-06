/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 1998, 2000, 2003 Free Software Foundation, Inc.

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

#include <sys/syscall.h>
#include <sysdep.h>
	.text

.globl __mmap
ENTRY (__mmap)

#ifdef PIC
.pic
#endif

   // reserve space for r0, r1
#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP)
#else
   addi  $sp, $sp, -8
#endif
   // change to units  of the  system  page  size
   srli $r5, $r5, 0xc

   syscall SYS_ify(mmap2)
#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP)
#else
   addi  $sp, $sp, 8
#endif

   /* r0 is < -4096 if there was an error.  */
   bgez $r0, 1f
   sltsi $r1,$r0,-4096
   beqz $r1,2f

1:
   ret
2:
#ifdef PIC
#ifdef __NDS32_N1213_43U1H__
	! save lp
	addi	$r2,	$lp,	0

	! set r1 as gp
	jal	1b
	sethi	$r1,	hi20(_GLOBAL_OFFSET_TABLE_)
	ori	$r1,	$r1,	lo12(_GLOBAL_OFFSET_TABLE_+4)
	add	$r1,	$lp,	$r1

	! restore lp
	addi	$lp,	$r2,	0

	! r15=SYSCALL_ERROR@PLT
	sethi $r15, hi20(SYSCALL_ERROR@PLT)
	ori	$r15,	$r15, lo12(SYSCALL_ERROR@PLT)
	add	$r15, $r15, $r1

	! jump to SYSCALL_ERROR
	jr		$r15
#else
	! set r1 as gp
	mfusr $r15, $PC
	sethi	$r1,	hi20(_GLOBAL_OFFSET_TABLE_+4)
	ori	$r1,	$r1,	lo12(_GLOBAL_OFFSET_TABLE_+8)
	add	$r1,	$r1,	$r15

	! r15=SYSCALL_ERROR@PLT
	sethi $r15, hi20(SYSCALL_ERROR@PLT)
	ori	$r15,	$r15, lo12(SYSCALL_ERROR@PLT)
	add	$r15, $r15, $r1

	! jump to SYSCALL_ERROR
	jr		$r15
#endif
#else
	j SYSCALL_ERROR
#endif

ret

PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
libc_hidden_def(mmap)
