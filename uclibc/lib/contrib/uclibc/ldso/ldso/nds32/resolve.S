/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP)
# define STACK_PUSH
# define STACK_POP
#else
# define STACK_PUSH     addi   $sp,    $sp,    -24
# define STACK_POP      addi   $sp,    $sp,    24
#endif

 .text
 .align 4      ! 16 byte boundary
 .globl _dl_linux_resolve
 .type _dl_linux_resolve,#function
 .pic

_dl_linux_resolve:
	! we get called with
	! 	lp contains the return address from this call
	!	r16 contains offset to target reloc entry
	!	r17 contains GOT[1] (identity of taget lib)
	!  ta is GOT[2] (starting address of this function)

	! save arguments r0 - r5 and gp, lp
	smw.adm	$r0,	[$sp],	$r5,	6

	! init gp
#ifdef __NDS32_N1213_43U1H__
	sethi	$gp,	HI20(_GLOBAL_OFFSET_TABLE_+4)
	ori	$gp,	$gp,	LO12(_GLOBAL_OFFSET_TABLE_+8)
	add	$gp,	$ta,	$gp
#else
	mfusr	$ta,	$PC
	sethi	$gp,	HI20(_GLOBAL_OFFSET_TABLE_+4)
	ori	$gp,	$gp,	LO12(_GLOBAL_OFFSET_TABLE_+8)
	add	$gp,	$ta,	$gp
#endif

	! #ifdef __NDS32_ABI_1__
	! adjust stack
	!addi	$sp,	$sp,	-24
	STACK_PUSH
	! #endif

	! set arguments
	addi	$r0,	$r17,	0
	!addi	$r1,	$r16,	0
	slli    $r1,    $r16,   2
	slli    $r16,   $r16,   3
	add     $r1,    $r1,    $r16

	! comment out profiling argument
	!addi	$r2,	$lp,	0

	! call fixup routine
	bal	_dl_linux_resolver@PLT

	! save the return
	addi	$ta,	$r0,	0

	! #ifdef __NDS32_ABI_1__
	! adjust sp
	!addi	$sp,	$sp,	24
	STACK_POP
	! #endif

	! reload registers
	lmw.bim	$r0,	[$sp],	$r5,	6

	! jump to the newly found address
	jr		$ta

.size _dl_linux_resolve, .-_dl_linux_resolve
