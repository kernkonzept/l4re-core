/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <sys/asm.h>
#include <sys/regdef.h>

	.text

ARGSZ	= 0
LOCALSZ	= 2 # save ra, gp

FRAMESZ	= (((ARGSZ + LOCALSZ) * SZREG) + ALSZ) & ALMASK
RAOFF	= FRAMESZ - (1 * SZREG)
GPOFF	= FRAMESZ - (2 * SZREG)

NESTED(thread_page_in_handler_stub, FRAMESZ, ra)
	SETUP_GP
	PTR_SUBU sp, FRAMESZ
	SETUP_GP64(GPOFF, thread_page_in_handler_stub)
	SAVE_GP(GPOFF)
	REG_S	ra, RAOFF(sp)

	jal thread_page_in_handler_impl

	REG_L	ra, RAOFF(sp)
	RESTORE_GP64
	PTR_ADDU sp, FRAMESZ
	j	ra
END(thread_page_in_handler_stub)
