/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sysdep.h>

	.text
#ifdef PIC
	.pic
#endif
	.align 2


#ifdef PIC
#ifdef __NDS32_N1213_43U1H__
1:
	ret
99:
	addi 	$r2, $lp, 0
	jal 	1b
	sethi 	$r1, hi20(_GLOBAL_OFFSET_TABLE_)
	ori 	$r1, $r1, lo12(_GLOBAL_OFFSET_TABLE_+4)
	add 	$r1, $lp, $r1
	addi 	$lp, $r2, 0
#else		/* !__NDS32_N1213_43U1H__ */
99:
	mfusr 	$r15, $PC
	sethi 	$r1, hi20(_GLOBAL_OFFSET_TABLE_ + 4)
	ori 	$r1, $r1, lo12(_GLOBAL_OFFSET_TABLE_ + 8)
	add 	$r1, $r15, $r1
#endif		/* end of __NDS32_N1213_43U1H__ */
	sethi 	$r15, hi20(__syscall_error@PLT)
	ori 	$r15, $r15, lo12(__syscall_error@PLT)
	add 	$r15, $r15, $r1
	jr 		$r15
#else		/* !PIC	*/
99:
	j __syscall_error
#endif		/* end of PIC */

#ifdef PIC	
	.pic 
#endif

	.align 2
	.globl syscall
	.func syscall
	.type syscall, @function

syscall:
	syscall __NR_syscall
	bgez $r0, 2f
	sltsi $r1, $r0, -4096; 
	beqz $r1, 99b; 
2:
	ret
	.endfunc 
	.size syscall, .-syscall
