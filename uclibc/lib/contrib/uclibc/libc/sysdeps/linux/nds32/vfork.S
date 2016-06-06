/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 1999, 2002, 2003 Free Software Foundation, Inc.
   Contributed by Philip Blundell <philb@gnu.org>.

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

#define _ERRNO_H    1

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)
#ifdef PIC
.pic
#endif

#ifdef __NR_vfork

    syscall __NR_vfork
    bltz $r0, 2f
1:
    ret
2:
    sltsi $r1, $r0, -4096
    bnez $r1, 1b;

#  ifdef PIC
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
	#else
		! set r1 as gp
		mfusr $r15, $PC
		sethi	$r1,	hi20(_GLOBAL_OFFSET_TABLE_+4)
		ori	$r1,	$r1,	lo12(_GLOBAL_OFFSET_TABLE_+8)
		add	$r1,	$r1,	$r15
	#endif

	! r15=C_SYMBOL_NAME(__syscall_error)@PLT
	sethi $r15, hi20(C_SYMBOL_NAME(__syscall_error)@PLT)
	ori	$r15,	$r15, lo12(C_SYMBOL_NAME(__syscall_error)@PLT)
	add	$r15, $r15, $r1

	! jump to SYSCALL_ERROR
	jr		$r15
#  else
	j C_SYMBOL_NAME(__syscall_error)
#  endif

#else
# error "__NR_vfork not available"
#endif

PSEUDO_END (__vfork)
weak_alias (__vfork, vfork)
libc_hidden_def(vfork)
