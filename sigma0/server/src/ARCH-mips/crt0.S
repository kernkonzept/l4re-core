/*
 * Copyright (C) 2013 Imagination Technologies Ltd.
 * Author: Yann Le Du <ledu@kymasys.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <sys/asm.h>

.section .text.init, "ax"
.type __start, @function
.globl __start
.set noreorder
__start:
	SETUP_GPX($0)
	SETUP_GPX64($25, $0)
	PTR_LA	$29, crt0_stack_high
	PTR_LA	$25, init
	PTR_SUBU  $29, (NARGSAVE * SZREG)
	jalr $25
	  nop
1:
	b   1b

.section ".bss", "aw"
.balign 16
.global crt0_stack_low
crt0_stack_low:
	.space 4096
.global crt0_stack_high
crt0_stack_high:
