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
	adrp    x9, crt0_stack_high
	add     x9, x9, :lo12:crt0_stack_high

	mov	sp, x9

#ifdef CONFIG_BID_PIE
	/* Save off the kip pointer */
	mov     x19, x0

	/* Calculate load address... subtract link address from real one */
	adrp    x0, :got:_start
	ldr     x0, [x0, #:got_lo12:_start]
	adr     x1, _start
	sub     x0, x1, x0

	/* Do relocations */
	bl reloc_static_pie

	/* restore kip pointer */
	mov     x0, x19
#endif

	adr	x30, _exit
	b	init

.section ".bss"

	.balign	16
	.global crt0_stack_low
crt0_stack_low:
	.space	2048
	.global crt0_stack_high
crt0_stack_high:
