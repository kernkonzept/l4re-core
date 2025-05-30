/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

.section .text.init, "ax"
.type _start,#function
.globl _start
_start:
	ta 0

.LCstack: .word crt0_stack_high
.LCexit:  .word _exit

.section ".bss"

	.balign	16
	.global crt0_stack_low
crt0_stack_low:
	.space	2048
	.global crt0_stack_high
crt0_stack_high:
