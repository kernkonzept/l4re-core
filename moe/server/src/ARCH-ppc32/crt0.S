/**
 * \file
 * \brief	Startup code
 *
 * \date	06/03/2003
 * \author	Frank Mehnert <fm3@os.inf.tu-dresden.de>
 * \author	Sebastian Sumpf <sumpf@os.inf.tu-dresden.de> */

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

.section .text, "ax"
.type _real_start, @function
.globl _real_start
_real_start:
  lis   %r1, _stack_top@ha
  addi  %r1, %r1, _stack_top@l
  b     _start

/*--- my stack -------------------------------------*/
.section ".bss", "aw"
.global _stack_bottom
_stack_bottom:
	.space 0x4000
.global _stack_top
_stack_top:
