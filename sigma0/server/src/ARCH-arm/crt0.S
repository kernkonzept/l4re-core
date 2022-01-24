/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
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

	ldr a1, .L_GOT          /* Get value at .L_GOT + 0  (offset to GOT)*/
	adr a2, .L_GOT          /* Get address of .L_GOT */
	ldr a3, .L_GOT+4        /* Get value of _start(GOT) stored in .L_GOT */
	adr a4, _start          /* Get address of _start after relocation (changes to pc - ~30 or so) */
	add a1, a1, a2          /* Calculate where the GOT is */
	ldr a2, [a1, a3]        /* GOT + _start(GOT) = offset of _start from begin of file */
	sub a1, a4, a2          /* Current addr of _start - offset from beginning of file = load addr */
	bl reloc_static_pie

	mov	a1, r4          /* restore kip pointer */
#endif

	b	init
.LCstack: .word (crt0_stack_high - _start)
.LCexit:  .word (_exit - _start)
#ifdef CONFIG_BID_PIE
.L_GOT:
	.word _GLOBAL_OFFSET_TABLE_ - .L_GOT
	.word _start(GOT)
#endif

.section ".bss"

	.balign	16
	.global crt0_stack_low
crt0_stack_low:
	.space	2048
	.global crt0_stack_high
crt0_stack_high:
