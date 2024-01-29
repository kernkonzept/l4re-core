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

#ifdef __mips64
	#define ELF_HDR_E_PHOFF 0x20
	#define ELF_HDR_E_PHNUM 0x38
#else
	#define ELF_HDR_E_PHOFF 0x1c
	#define ELF_HDR_E_PHNUM 0x2c
#endif

.section .text, "ax"

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
 * This is required for C++ exceptions to work in relocated (PIE) executables
 * and with libunwind (LLVM)!
 *
 * ATTENTION: the following code assumes that the executable starts directly
 * with the ELF header! For regularly loaded executables the aux vector is
 * provided by libloader, which can work with any elf file.
 */
.type _real_start, @function
.globl _real_start
.set noreorder
_real_start:
	SETUP_GPX($0)
	SETUP_GPX64($25, $0)
	/* Mind the argc/argv/env/auxv that needs to be on the stack! */
	PTR_LA	$29, _stack_top - (8 * SZREG)

	PTR_LA	$8, __executable_start

	li		$9, 3			// AT_PHDR
	LONG_S		$9, 3*SZREG($29)
	LONG_L		$9, ELF_HDR_E_PHOFF($8)	// e_phoff
	LONG_ADDU	$9, $8, $9
	LONG_S		$9, 4*SZREG($29)

	li		$9, 5			// AT_PHNUM
	LONG_S		$9, 5*SZREG($29)
	lbu		$9, ELF_HDR_E_PHNUM($8)	// e_phnum
	lbu		$10, ELF_HDR_E_PHNUM + 1($8)
#ifdef __MIPSEB
	sll		$9, $9, 8
#else
	sll		$10, $10, 8
#endif
	LONG_ADDU	$9, $9, $10
	LONG_S		$9, 6*SZREG($29)

	PTR_LA	$25, __start
	jalr $25
	   nop
1:
	b   1b

/*--- my stack -------------------------------------*/
.section ".bss", "aw"
	.balign 16
.global _stack_bottom
_stack_bottom:
	.space 0x4000
.global _stack_top
_stack_top:
