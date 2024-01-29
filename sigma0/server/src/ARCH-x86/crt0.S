/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#define __ASSEMBLY__
#include <l4/sys/compiler.h>
	
	.section .text

ENTRY(__crt_dummy__)
ENTRY(_start)
ENTRY(start)

	test	%ecx, %ecx
	jne 1f
	popl	%ecx		/* ptr to kernel info page is on init stack */
1:
	leal	_stack_top-12, %esp
	pushl	%ecx		/* push args to init() */

	/* The i386 System V ABI requires the stack pointer to be 16-byte
	 * aligned just before the call instruction is executed.
         * 'push' + 'jmp' emulate 'call'. */
	pushl	$_exit
	jmp	init

ENTRY(__main)
	ret
	
	.bss

	.balign	16
	.space	2048
_stack_top:
