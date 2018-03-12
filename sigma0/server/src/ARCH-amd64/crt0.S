/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#define __ASSEMBLY__
#include <l4/sys/compiler.h>
	
	.section .text.init

ENTRY(__crt_dummy__)
ENTRY(_start)
ENTRY(start)

	test	%rdi, %rdi
	jne	1f
	pop	%rdi		/* ptr to kernel info page is on init stack */
1:
	lea	_stack,%rsp	/* reset stack */
	push	$_exit
	jmp	init

ENTRY(__main)
	ret
	
	.bss

	.space	4096
ENTRY(_stack)
