/**
 * \file
 * \brief	Startup code
 *
 * \date	06/03/2003
 * \author	Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#define __ASSEMBLY__
#include <l4/sys/compiler.h>

/*--- .text (program code) -------------------------*/
	.text

	.globl	_real_start
_real_start:
	cmp	x1, #0
	b.eq	1f
	adr	x9, __l4_sys_syscall
	ldr	x2, [x9]
	cmp	x1, x2
	b.eq	1f
	str	x1, [x9]
	dmb ish
	isb
	dc cvac, x9
	ic ivau, x9
	isb
1:
	ldr 	x9, .LCstack
	mov 	sp, x9
	b	_start

/* Mind the argc that is popped off the stack! */
.LCstack: .8byte _stack_top-16

/* This is not a normal function and must not clobber any register.
 * This is a special syscall stub -- do not add any code! */
.global __l4_sys_syscall
.type __l4_sys_syscall, @function
__l4_sys_syscall:
#ifdef CONFIG_KERNEL_CPU_VIRT
	hvc #0
#else
	svc #0
#endif
	ret
.size __l4_sys_syscall, . - __l4_sys_syscall


/*--- my stack -------------------------------------*/
	.bss
	.align 4
_stack_bottom:
	.space 0x4000
_stack_top:
