/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#include <sys/syscall.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/*
       int clone(int (*fn)(void *), void *child_stack, int flags, void *arg);
       _syscall2(int, clone, int, flags, void *, child_stack)
*/

ENTRY(__clone)
#ifdef PIC
	/* set GP register to parent only, cause child's $SP will be $r1. */
	pushm	$fp, $gp
#ifndef __NDS32_N1213_43U1H__
	mfusr $r15, $PC
#endif
	sethi	$gp,	hi20(_GLOBAL_OFFSET_TABLE_+4)
	ori	$gp,	$gp,	lo12(_GLOBAL_OFFSET_TABLE_+8)
	add	$gp,	$gp,	$r15
#endif
        /* sanity check arguments.  */
        beqz    $r0, 1f
        bnez    $r1, 2f

1:
        movi    $r0, -EINVAL
5:
#ifdef PIC
		  /* restore GP register, only in parent's stack */
		  popm	$fp, $gp
        la      $r15, C_SYMBOL_NAME(__syscall_error@PLT)
        jr      $r15
#else
        b       C_SYMBOL_NAME(__syscall_error)
#endif

2:
	/* Child's $SP will be $r1, push to child's stack only. */
	addi	$r1, $r1, -4
        swi.p   $r3, [$r1], -4  ! arg
        swi     $r0, [$r1]  ! fn

        /* do the system call */
        or      $r0, $r2, $r2   ! move r0, r2
        __do_syscall(clone)
	!syscall (__NR_clone)
        beqz    $r0, 4f
        bltz    $r0, 5b

        ! parent
#ifdef PIC
	/* restore GP register, only in parent's stack */
	popm	$fp, $gp
#endif
        ret

4:
	/* Only in child's stack. */
        pop     $r1  ! fn
        pop     $r0  ! arg
#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP)
#else
	addi	$sp, $sp, -24
#endif
	! use r15 in case _exit is PIC
#ifdef __NDS32_N1213_43U1H__
        or      $r15, $r1, $r1   ! move r15, r2
#endif
        bral    $r1
#if defined(NDS32_ABI_2) || defined(NDS32_ABI_2FP)
#else
	addi	$sp, $sp, 24
#endif
	! use r15 in case _exit is PIC
#ifdef PIC
        la      $r15, C_SYMBOL_NAME(_exit@PLT)
        jr      $r15
#else
        b       C_SYMBOL_NAME(_exit)
#endif


PSEUDO_END (__clone)
weak_alias (__clone, clone)
