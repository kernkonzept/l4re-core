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
 * int setcontext (const ucontext_t *ucp)
 */
ENTRY(__setcontext)
	get $r16 = $ra
	addd $r12 = $r12, -32
	;;
	/* Save ucp pointer  and $ra on the stack because we can't trash
	 * any callee saved registers in case __setcontext returns */
	sd 16[$r12] = $r16
	;;
	sd 24[$r12] = $r0
	;;
	/* Bring back the signal status. */
	make $r0 = SIG_SETMASK
	addd $r1 = $r0, UCONTEXT_SIGMASK
	make $r2 = 0
	;;
	/* sigprocmask(SIG_SETMASK, &(ucontext->uc_sigmask), NULL) */
	call sigprocmask
	;;
	/* Check return value of sigprocmask */
	cb.deqz $r0 ? 1f
	/* Normally __setcontext does not return. But in case of an error it
	 * returns with -1 and an appropriate errno */
	ld $r16 = 16[$r12]
	;;
	set $ra = $r16
	addd $r12 = $r12, 32
	;;
	goto __syscall_error
	;;
1:
	/* Get back the ucp pointer */
	ld $r16 = 24[$r12]
	/* Reset the stack pointer (we can trash $ra here, because we will
	 * never return to after __setcontext from this point onwards) */
	addd $r12 = $r12, 32
	;;
	/* Restore callee saved registers */
	lq $r18r19 = (MCONTEXT_Q16 + 16)[$r16]
	;;
	/* Setup $r20, $r21 for the __startcontext to work */
	lo $r20r21r22r23 = MCONTEXT_Q20[$r16]
	;;
	lo $r24r25r26r27 = MCONTEXT_Q24[$r16]
	;;
	lo $r28r29r30r31 = MCONTEXT_Q28[$r16]
	;;
	/* Restore special registers */
	lo $r40r41r42r43 = MCONTEXT_LC_LE_LS_RA[$r16]
	;;
	/* Now load argument registers */
	lo $r0r1r2r3 = MCONTEXT_Q0[$r16]
	set $lc = $r40
	;;
	lo $r4r5r6r7 = MCONTEXT_Q4[$r16]
	set $le = $r41
	;;
	lo $r8r9r10r11 = MCONTEXT_Q8[$r16]
	set $ls = $r42
	;;
	/* Restore $sp */
	ld $r12 = MCONTEXT_Q12[$r16]
	/* Restore $ra which points to the $ra set by __getcontext or
	 * to__startcontext if __makecontext was called in between */
	set $ra = $r43
	;;
	ld $r40 = MCONTEXT_CS_SPC[$r16]
	;;
	ld $r14 = (MCONTEXT_Q12 + 16)[$r16]
	set $cs = $r40
	;;
	ret
	;;
END(setcontext)
weak_alias(__setcontext, setcontext)

ENTRY(__startcontext)
	icall $r21
	;;
	copyd $r0 = $r20
	/* Check if the new context is 0 if so just call _exit */
	cb.deqz $r20 ? 1f
	;;
	goto __setcontext
	;;
	/* This should never be reached otherwise kill the thread */
1:	goto _exit
	;;
END(__startcontext)
