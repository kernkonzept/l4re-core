/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

.section .text.init,#alloc,#execinstr
.type _start,#function
.globl _start
_start:
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
	adr	x9, crt0_stack_high
	mov	sp, x9
	adr	x30, _exit
	b	init

/* This is not a normal function and must not clobber any register.
 * This is a special syscall stub -- do not add any code! */
.global __l4_sys_syscall
.type __l4_sys_syscall, @function
__l4_sys_syscall:
	svc #0
	ret
.size __l4_sys_syscall, . - __l4_sys_syscall

.section ".bss"

	.align	4
	.global crt0_stack_low
crt0_stack_low:
	.space	2048
	.global crt0_stack_high
crt0_stack_high:
