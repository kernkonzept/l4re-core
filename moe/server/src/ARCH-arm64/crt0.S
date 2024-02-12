/**
 * \file
 * \brief	Startup code
 *
 * \date	06/03/2003
 * \author	Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#define __ASSEMBLY__
#include <l4/bid_config.h>
#include <l4/sys/compiler.h>

/*--- .text (program code) -------------------------*/
	.text


/*
 * Provide the following stack layout to uclibc:
 *
 *	0x00	0	(argc)
 *	0x08	NULL	(argv final NULL)
 *	0x10	NULL	(env final NULL)
 *			auxv[]
 *	0x18	3		AT_PHDR
 *	0x20	?		<ptr to PHDRs>
 *	0x28	5		AT_PHNUM
 *	0x30	?		<number of PHDRs>
 *	0x38	NULL		(auxv termination)
 *
 * This is required for C++ exceptions to work in relocated (PIE) executables!
 *
 * ATTENTION: the following code assumes that the executable starts directly
 * with the ELF header! For regularly loaded executables the aux vector is
 * provided by libloader, which can work with any elf file.
 */
	.globl	_real_start
_real_start:
	ldr 	x9, .LCstack
	adr	x10, _real_start
	add	x9, x9, x10
	mov 	sp, x9

	adr	x16, __executable_start

	mov	x17, #3			// AT_PHDR
	str	x17, [sp, #0x18]
	ldr	x17, [x16, #0x20]	// e_phoff
	add	x17, x16, x17
	str	x17, [sp, #0x20]

	mov	x17, #5			// AT_PHNUM
	str	x17, [sp, #0x28]
	ldurh	w17, [x16, #0x38]	// e_phnum
	str	x17, [sp, 0x30]

	b	_start

/* Mind the argc/argv/env/auxv that needs to be on the stack! */
.LCstack: .8byte (_stack_top - _real_start - 64)


/*--- my stack -------------------------------------*/
	.bss
	.balign 16
_stack_bottom:
	.space 0x4000
_stack_top:
