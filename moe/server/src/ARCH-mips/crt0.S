/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License 2.  See the file "COPYING-GPL-2" in the main directory of this
 * archive for more details.
 *
 * Copyright (C) 2013 Imagination Technologies Ltd.
 * Author: Yann Le Du <ledu@kymasys.com>
 */

/**
 * \file
 * \brief	Startup code
 */

#include <sys/asm.h>

/*--- .text (program code) -------------------------*/

.section .text, "ax"
.type _real_start, @function
.globl _real_start
.set noreorder
_real_start:
	.cpload $25 /* load gp */
	SETUP_GPX64($25, $0)
	PTR_LA	$29, _stack_top
	PTR_LA	$25, __start
	PTR_SUBU  $29, (NARGSAVE * SZREG)
	jalr $25
	   nop
1:
	b   1b

/*--- my stack -------------------------------------*/
.section ".bss", "aw"
.global _stack_bottom
_stack_bottom:
	.space 0x4000
.global _stack_top
_stack_top:
