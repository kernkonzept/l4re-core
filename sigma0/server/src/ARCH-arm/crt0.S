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
	adr	r12, _start
	ldr	sp, .LCstack
	add	sp, sp, r12
	ldr	lr, .LCexit
	add	lr, lr, r12
	b	init
.LCstack: .word (crt0_stack_high - _start)
.LCexit:  .word (_exit - _start)

.section ".bss"

	.balign	16
	.global crt0_stack_low
crt0_stack_low:
	.space	2048
	.global crt0_stack_high
crt0_stack_high:
