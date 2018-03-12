/*!
 * \file	crt0_l4env_freebsd.S
 * \brief	Startup code for L4env applications for l4env_freebsd mode
 *
 * \date	06/03/2003
 * \author	Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#define __ASSEMBLY__
#include <l4/sys/compiler.h>

/*--- .text (program code) -------------------------*/
	.text

	.globl	_start
_start:
	/* Clear the base pointer so that stack backtraces will work.  */
	xor	%rbp,%rbp

	pop	%rdi
	mov	%rsp, %rsi

	/* Jump into C code (init backends, call constructors, call main). */
	call	__main

	/* We should never get here since l4env_startup() does not return */
1:	int	$3
	jmp	2f
	.ascii	"__main() returned."
2:	jmp	1b


/*--- .ctors (array of pointers to constructors) ---*/
	.section .ctors,"a",@progbits
	.p2align(2)
ctors:


/*--- .dtors (array of pointers to destructors) ----*/
	.section .dtors,"a",@progbits
	.p2align(2)
dtors:


/*--- .anno (annoations used by OSKit) --------------*/
	.section .anno,"aw",@progbits
	.p2align(4)
	.globl	__ANNO_START__
__ANNO_START__:


/*--- .data (initialized data) ---------------------*/
	.data
	
	.globl	__dso_handle
__dso_handle:
	.long	0


/*--- .eh_frame (exception frames) -----------------*/
	.section .eh_frame,"a"
	.globl	__EH_FRAME_BEGIN__
__EH_FRAME_BEGIN__:

