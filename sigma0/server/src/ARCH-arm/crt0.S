/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#define __ASSEMBLY__
#include <l4/bid_config.h>

.section .text.init, "ax"
.type _start,#function
.globl _start
_start:
	adr	r12, _start
	ldr	sp, .LCstack
	add	sp, sp, r12
	ldr	lr, .LCexit
	add	lr, lr, r12

#ifdef CONFIG_BID_PIE
	mov	r4, a1          /* save kip pointer */

	adr	a1, .L__ehdr_start_off
	ldr	a2, .L__ehdr_start_off
	add	a1, a1, a2
	bl	reloc_static_pie

	mov	a1, r4          /* restore kip pointer */
#endif

	b	init
.LCstack: .word (crt0_stack_high - _start)
.LCexit:  .word (_exit - _start)
#ifdef CONFIG_BID_PIE
.L_GOT:
	.word _GLOBAL_OFFSET_TABLE_ - .L_GOT
	.word _start(GOT)
.L__ehdr_start_off:
	.word __ehdr_start - .L__ehdr_start_off
#endif

.section ".bss"

	.balign	16
	.global crt0_stack_low
crt0_stack_low:
	.space	2048
	.global crt0_stack_high
crt0_stack_high:
