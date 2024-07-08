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

/* int swapcontext (ucontext_t *oucp, const ucontext_t *ucp) */

ENTRY(swapcontext)
	/* Make space on the stack to save all caller saved registers */
	get $r16 = $ra
	addd $r12 = $r12, -32
	;;

	/* Save $ra on the stack */
	sd (24)[$r12] = $r16
	;;
	/* Save the arguments of swapcontext on the stack */
	sq (8)[$r12] = $r0r1
	;;
	call __getcontext
	;;

	/* Save the return value of __getcontext in $r17 */
	copyd $r17 = $r0

	/* Restore $ra */
	ld $r16 = (24)[$r12]
	;;
	/* Restore arguments */
	lq $r0r1 = (8)[$r12]
	/* Readjust the stack pointer */
	addd $r12 = $r12, 32
	/* Also restore $ra */
	set $ra = $r16
	;;

	/* Exit if getcontext() failed */
	cb.deqz $r17 ? 1f
	;;

	/* Set the return value set by __syscall_error in __getcontext */
	copyd $r0 = $r17
	ret
	;;
1:
	/* Store the $sp and $ra in the context to be saved */
	sd (MCONTEXT_Q12)[$r0] = $r12
	;;
	sd (MCONTEXT_LC_LE_LS_RA + 24)[$r0] = $r16
	copyd $r0 = $r1
	goto __setcontext
	;;
END(swapcontext)
