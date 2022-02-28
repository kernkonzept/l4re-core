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
	adr	x9, crt0_stack_high
	mov	sp, x9
	adr	x30, _exit
	b	init

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

.section ".bss"

	.balign	16
	.global crt0_stack_low
crt0_stack_low:
	.space	2048
	.global crt0_stack_high
crt0_stack_high:
