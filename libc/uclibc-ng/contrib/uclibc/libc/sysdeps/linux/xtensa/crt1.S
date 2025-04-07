/* Startup code compliant to the ELF Xtensa ABI.
   Copyright (C) 2001, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <features.h>

#if defined(__FDPIC__)

/* This is the canonical entry point, usually the first thing in the text
   segment.  When the entry point runs, most register values are unspecified,
   except for:

	a6	Address of .dynamic section
	a5	Interpreter map
	a4	Executable map

	a2	Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

	a1	The stack (i.e., a1+16) contains the arguments and environment:
		a1+0			argc
		a1+4			argv[0]
		...
		a1+(4*argc)		NULL
		a1+(4*(argc+1))		envp[0]
		...
					NULL
 */
	.text
	.align	4
	.literal_position
	.global	_start
	.type	_start, @function
_start:
#if defined(__XTENSA_CALL0_ABI__)

	.begin	no-transform
	call0	1f
2:
	.end	no-transform
	.align	4
	.literal_position
1:
	movi	a15, 2b
	sub	a15, a0, a15

	mov	a12, a4
	mov	a13, a5
	mov	a14, a6
	mov	a2, a4
	movi	a3, __ROFIXUP_LIST__
	add	a3, a3, a15
	movi	a4, __ROFIXUP_END__
	add	a4, a4, a15
	movi	a0, __self_reloc
	add	a0, a0, a15
	callx0	a0

	mov	a11, a2
	movi	a2, main@GOTOFFFUNCDESC
	add	a2, a2, a11
	l32i	a3, sp, 0	/* argc */
	addi	a4, sp, 4	/* argv */
	/* a5 is either 0 when static or set by the RTLD to the rtld_fini */
	mov	a7, a13
	/* unused stack_end argument is what used to be argc */
	movi	a5, _init@GOTOFFFUNCDESC
	add	a5, a5, a11
	movi	a6, _fini@GOTOFFFUNCDESC
	add	a6, a6, a11

	movi	a0, __uClibc_main@GOTOFFFUNCDESC
	add	a0, a0, a11
	l32i	a11, a0, 4
	l32i	a0, a0, 0
	callx0	a0
	ill

#else
#error Unsupported Xtensa ABI
#endif

#else /* defined(__FDPIC__) */

#ifndef __UCLIBC_CTOR_DTOR__
        .weak _init
        .weak _fini
#endif

/* This is the canonical entry point, usually the first thing in the text
   segment.  When the entry point runs, most register values are unspecified,
   except for:

	a2	Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

	a1	The stack (i.e., a1+16) contains the arguments and environment:
		a1+0			argc
		a1+4			argv[0]
		...
		a1+(4*argc)		NULL
		a1+(4*(argc+1))		envp[0]
		...
					NULL

  Setup parameters accordingly (for a call4).  See function prototype
  from sysdeps/generic/libc-start.c

	a6 = *main
	a7 = argc
	a8 = ubp_av
	a9 = *init
	a10 = *fini
	a11 = *rtld_fini
	[sp+0] = stack_end
 */

	.text
	.align	4
	.literal_position
	.global	_start
	.type	_start, @function
_start:
#ifdef L_rcrt1
	.begin	no-transform
	call0	1f
.Lret_addr:
	.end	no-transform
	.align	4
1:
#endif
#if defined(__XTENSA_WINDOWED_ABI__)
#ifdef L_rcrt1
	movi	a6, .Lret_addr
	sub	a6, a0, a6
	movi	a0, 0
	movi	a4, reloc_static_pie
	add	a4, a4, a6
	callx4	a4
#else
	/* Clear a0 to obviously mark the outermost frame.  */
	movi	a0, 0
#endif

	/* Load up the user's main function.  */
	movi	a6, main

	/* Extract the arguments as encoded on the stack and set up
	   the arguments for `main': argc, argv.  envp will be determined
	   later in __uClibc_main.  */
	l32i	a7, a1, 0	/* Load the argument count.  */
	addi	a8, a1, 4	/* Compute the argv pointer.  */

	/* Push address of our own entry points to .fini and .init.  */
	movi	a9, _init
	movi	a10, _fini

	/* Setup the shared library termination function.  */
	mov	a11, a2

	/* Provide the highest stack address to the user code (for stacks
	   which grow downwards).  Note that we destroy the stack version
	   of argc here.  */
	s32i	a1, a1, 0

	/* Call the user's main function, and exit with its value.
	   But let the libc call main.    */
	movi	a4, __uClibc_main
	callx4	a4
#elif defined(__XTENSA_CALL0_ABI__)
#ifdef L_rcrt1
	mov	a12, a2
	movi	a2, .Lret_addr
	sub	a2, a0, a2
	movi	a0, reloc_static_pie
	add	a0, a0, a2
	callx0	a0
	mov	a7, a12
#else
	/* Setup the shared library termination function.  */
	mov	a7, a2
#endif

	/* Load up the user's main function.  */
	movi	a2, main

	/* Extract the arguments as encoded on the stack and set up
	   the arguments for `main': argc, argv.  envp will be determined
	   later in __uClibc_main.  */
	l32i	a3, a1, 0	/* Load the argument count.  */
	addi	a4, a1, 4	/* Compute the argv pointer.  */

	/* Push address of our own entry points to .fini and .init.  */
	movi	a5, _init
	movi	a6, _fini

	/* Provide the highest stack address to the user code (for stacks
	   which grow downwards).  Note that we destroy the stack version
	   of argc here.  */
	s32i	a1, a1, 0

	/* Call the user's main function, and exit with its value.
	   But let the libc call main.    */
	movi	a0, __uClibc_main
	callx0	a0
#else
#error Unsupported Xtensa ABI
#endif

	/* Crash if somehow `exit' does return.  */
	ill

	/* Define a symbol for the first piece of initialized data.  */
	.data
	.align	4
	.global	__data_start
__data_start:
	.long	0
	.weak	data_start
	data_start = __data_start
#endif /* defined(__FDPIC__) */
