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
 *	0x04	NULL	(argv final NULL)
 *	0x08	NULL	(env final NULL)
 *			auxv[]
 *	0x0c	3		AT_PHDR
 *	0x10	?		<ptr to PHDRs>
 *	0x14	5		AT_PHNUM
 *	0x18	?		<number of PHDRs>
 *	0x1c	NULL		(auxv termination)
 *
 * This is required for C++ exceptions to work in relocated (PIE) executables
 * and with libunwind (LLVM)!
 *
 * ATTENTION: the following code assumes that the executable starts directly
 * with the ELF header! For regularly loaded executables the aux vector is
 * provided by libloader, which can work with any elf file.
 */
	.globl	_real_start
_real_start:
	ldr 	sp, .LCstack
	adr	r12, _real_start
	add	sp, sp, r12

	ldr	r9, .LC__executable_start
	add	r9, r9, r12

	mov	r10, #3			// AT_PHDR
	str	r10, [sp, #0x0c]
	ldr	r10, [r9, #0x1c]	// e_phoff
	add	r10, r9, r10
	str	r10, [sp, #0x10]

	mov	r10, #5			// AT_PHNUM
	str	r10, [sp, #0x14]
	ldrh	r10, [r9, #0x2c]	// e_phnum
	str	r10, [sp, #0x18]

	b	_start

/* Mind the argc/argv/env/auxv that needs to be on the stack! */
.LCstack: .word (_stack_top - _real_start - 32)

.LC__executable_start: .word (__executable_start - _real_start)

/*--- my stack -------------------------------------*/
	.bss
	.balign 16
_stack_bottom:
	.space 0x4000
_stack_top:
