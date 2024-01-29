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
 * This is required for C++ exceptions to work in relocated (PIE) executables
 * and with libunwind (LLVM)!
 *
 * ATTENTION: the following code assumes that the executable starts directly
 * with the ELF header! For regularly loaded executables the aux vector is
 * provided by libloader, which can work with any elf file.
 */
	.globl	_real_start
_real_start:
	/* Clear the base pointer so that stack backtraces will work.  */
	xor	%rbp,%rbp
	/* Mind the argc/argv/env/auxv that needs to be on the stack! */
	lea	_stack_top-64(%rip),%rsp

	lea __executable_start(%rip),%rax

	movq	$3, 0x18(%rsp)		// AT_PHDR
	movq	0x20(%rax), %rdx	// e_phoff
	addq	%rax, %rdx
	movq	%rdx, 0x20(%rsp)

	movq	$5, 0x28(%rsp)		// AT_PHNUM
	movzwl	0x38(%rax), %edx	// e_phnum
	movq	%rdx, 0x30(%rsp)

	jmp _start@PLT
/*--- my stack -------------------------------------*/
	.bss
	.balign	16
_stack_bottom:
	.space 0x8000
_stack_top:

