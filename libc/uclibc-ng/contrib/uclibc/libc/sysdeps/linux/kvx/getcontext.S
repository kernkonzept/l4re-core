/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file "COPYING" in the main directory of this archive for
 * more details.
 *
 * Copyright (C) 2025 Kalray Inc.
 * Author(s): Julian Vetter <jvetter@kalrayinc.com>
 */

#include <sysdep.h>

#include "ucontext_i.h"

	.text

/*
 * int getcontext (ucontext_t *ucp)
 */
ENTRY(__getcontext)
	/* Save callee saved registers ($r14, $r18 - $r31)
	 * and some special registers */

	/* Save the entire occtuple, although we only need $sp and $r14 */
	so MCONTEXT_Q12[$r0] = $r12r13r14r15
	get $r4 = $lc
	;;
	/* Don't need to save veneer registers $r16 and $r17 */
	sq (MCONTEXT_Q16 + 16)[$r0] = $r18r19
	get $r5 = $le
	;;
	so MCONTEXT_Q20[$r0] = $r20r21r22r23
	get $r6 = $ls
	;;
	so MCONTEXT_Q24[$r0] = $r24r25r26r27
	get $r7 = $ra
	;;
	so MCONTEXT_Q28[$r0] = $r28r29r30r31
	get $r8 = $cs
	;;
	so MCONTEXT_LC_LE_LS_RA[$r0] = $r4r5r6r7
	/* Save ucp and $ra in callee saved registers because below we need to
	 * do a call to sigprocmask and afterwards we need to restore $ra. */
	copyd $r20 = $r0
	copyd $r21 = $r7
	;;
	sd MCONTEXT_CS_SPC[$r0] = $r8

	/* Prepare call to sigprocmask */
	make $r0 = SIG_BLOCK
	make $r1 = 0
	;;
	/* $r20 points to the ucontext */
	addd $r2 = $r20, UCONTEXT_SIGMASK
	/* Already set the return value for when this is called in the context
	 * of swapcontext. Because when this context returns it should look
	 * like as if swapcontext returned with 0. */
	sd MCONTEXT_Q0[$r20] = $r1
	;;
	/* sigprocmask(SIG_BLOCK, NULL, &(ucontext->uc_sigmask)) */
	call sigprocmask
	;;
	/* Restore $ra to point to the caller of this function. So,
	 * __getcontext will return with -1 (set by __syscall_error) and an
	 * appropriate errno */
	set $ra = $r21
	;;
	/* Check return value of sigprocmask */
	cb.deqz $r0 ? 1f
	;;
	goto __syscall_error
	;;
1:
	/* Restore used callee saved registers */
	lq $r20r21 = MCONTEXT_Q20[$r20]
	;;
	/* Set return value */
	make $r0 = 0
	ret
	;;
END(__getcontext)
weak_alias(__getcontext, getcontext)
