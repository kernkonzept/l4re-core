/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/* -*- c -*- */

/*#include <l4/sys/compiler.h>*/

.macro enter_kdebug
/*    	mov	lr, pc */
    	bla	#-0x00000020-8
    	b	1f
    	.ascii	" crtx "
    	.byte	0	
    	.align	4
    1:
.endm

.section .text.init, "ax"
.type _start, @function
.globl _start
_start:
     lwz %r3, 4(%r1)
     mr  %r4, %r1
     bl	__main
1:     
     b 1b

.section ".data"
	.global __dso_handle
__dso_handle:
	.long 0

