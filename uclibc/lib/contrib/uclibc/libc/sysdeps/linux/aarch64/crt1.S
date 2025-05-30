/* Copyright (C) 1995-2016 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* This is the canonical entry point, usually the first thing in the text
   segment.

   Note that the code in the .init section has already been run.
   This includes _init and _libc_init


   At this entry point, most registers' values are unspecified, except:

   x0		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp		The stack contains the arguments and environment:
		0(sp)			argc
		8(sp)			argv[0]
		...
		(8*argc)(sp)		NULL
		(8*(argc+1))(sp)	envp[0]
		...
					NULL
*/
/*
   For uClinux it looks like this:

        argc            argument counter (integer)
        argv            char *argv[]
        envp            char *envp[]
        argv[0]         program name (pointer)
        argv[1...N]     program args (pointers)
        argv[argc-1]    end of args (integer)
        NULL
        env[0...N]      environment variables (pointers)
        NULL

ARM register quick reference:

    Name    Alias        AArch64 Procedure Calling Standard Role

    x0                   argument 1 / integer result / scratch register / argc
    x1                   argument 2 / scratch register / argv
    x2                   argument 3 / scratch register / envp
    x3                   argument 4 / scratch register
    x4                   argument 5 / scratch register
    x5                   argument 6 / scratch register
    x6                   argument 7 / scratch register
    x7                   argument 8 / scratch register
    x8                   indirect result location 
    x9                   temporary register
    x10                  temporary register
    x11                  temporary register
    x12                  temporary register
    x13                  temporary register
    x14                  temporary register
    x15                  temporary register
    x16     ip0          intra procedure-call temporary register
    x17     ip1          intra procedure-call temporary register
    x18                  platform register / temporary register
    x19                  callee-saved
    x20                  callee-saved
    x21                  callee-saved
    x22                  callee-saved
    x23                  callee-saved
    x24                  callee-saved
    x25                  callee-saved
    x26                  callee-saved
    x27                  callee-saved
    x28                  callee-saved
    x29     fp           frame pointer
    x30     lr           link register
    sp                   stack pointer
*/

#include <features.h>

.text
	.globl	_start
	.type	_start,%function
	.type	_init,%function
	.type	_fini,%function
#ifndef __UCLIBC_CTOR_DTOR__
	.weak	_init
	.weak	_fini
#endif

_start:
	/* Create an initial frame with 0 LR and FP */
	mov	x29, #0
	mov	x30, #0

#ifdef L_rcrt1
	/* Save off the atexit pointer and syscall insn */
	mov     x19, x0
	mov     x20, x1

	/* "Calculate" load address. The link address of __ehdr_start is 0. */
	adrp    x0, __ehdr_start

	/* Do relocations */
	bl reloc_static_pie

	/* restore atexit pointer and syscall insn */
	mov     x0, x19
	mov     x1, x20
#endif

#ifdef __ARCH_USE_MMU__
	/* Pop argc off the stack and save a pointer to argv */
        /* XXX: We can't pop argc alone due to stack alignment, so we just read it */
        mov x2, sp
	ldr x1, [x2]
	add x2, x2, #0x8
#else
#error Unsupported yet
	/*
	 * uClinux/arm stacks look a little different from normal
	 * MMU-full Linux/arm stacks (for no good reason)
	 */
	/* pull argc and argv off the stack.  We are going to push 3
	 * arguments, so pop one here to maintain doubleword alignment.  */
	ldr a2, [sp], #4
	ldr a3, [sp]
#endif

	mov x6, x2	/* stack_end (unused) */
	mov x5, x0	/* rtld_fini */

#ifdef __PIC__
	adrp	x4, _fini
	add	x4, x4, :lo12:_fini
	adrp	x3, _init
	add	x3, x3, :lo12:_init
	adrp	x0, main
	add	x0, x0, :lo12:main

	/* __uClibc_main (main, argc, argv, init, fini, rtld_fini, stack_end) */
	/* __uClibc_main has 7 arguments, that is why we can pass them all in registers and not use the stack */
	/* Let the libc call main and exit with its return code.  */
	b __uClibc_main
#else
	/* Fetch address of fini */
	ldr x4, =_fini

	/* Set up the other arguments in registers */
	ldr x0, =main
	ldr x3, =_init

	/* __uClibc_main (main, argc, argv, init, fini, rtld_fini, stack_end) */

	/* Let the libc call main and exit with its return code.  */
	b __uClibc_main
#endif

	/* should never get here....*/
	bl	abort

	/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
