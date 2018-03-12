/*
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License 2.1.  See the file "COPYING-LGPL-2.1" in the main directory of
 * this archive for more details.
 *
 * Copyright (C) 2013 Imagination Technologies Ltd.
 * Author: Yann Le Du <ledu@kymasys.com>
 */

#define a0      $4
#define a1      $5
#define sp      $29

.section .text.init, "ax"
.type _start, @function
.globl _start
_start:
	lw    a0, 4(sp)
	move  a1, sp
	jal   __main
	nop
1:
	b    1b
