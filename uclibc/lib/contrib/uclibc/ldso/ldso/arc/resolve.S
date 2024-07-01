/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <asm.h>
#include <sysdep.h>
#include <sys/syscall.h>

; Save the registers which resolver could possibly clobber
; 	r0-r9: args to the function - symbol being resolved
; 	r10-r12 are already clobbered by PLTn, PLT0 thus neednot be saved

.macro	SAVE_CALLER_SAVED
	PUSHR_S	r0
	PUSHR_S	r1
	PUSHR_S	r2
	PUSHR_S	r3
	PUSHR	r4
	PUSHR	r5
	PUSHR	r6
	PUSHR	r7
	PUSHR	r8
	PUSHR	r9
	PUSHR_S	blink
.endm

.macro RESTORE_CALLER_SAVED_BUT_R0
	POPR	blink
	POPR	r9
	POPR	r8
	POPR	r7
	POPR	r6
	POPR	r5
	POPR	r4
	POPR_S	r3
	POPR_S	r2
	POPR_S	r1
.endm

; Upon entry, PLTn, which led us here, sets up the following regs
; 	r11 = Module info (tpnt pointer as expected by resolver)
;	r12 = PC of the PLTn itself - needed by resolver to find
;	      corresponding .rela.plt entry

ENTRY(_dl_linux_resolve)
	; args to func being resolved, which resolver might clobber
	SAVE_CALLER_SAVED

	mov_s 	r1, r12
	bl.d  	_dl_linux_resolver
	mov   	r0, r11

	RESTORE_CALLER_SAVED_BUT_R0
	j_s.d   [r0]    ; r0 has resolved function addr
	POPR_S	r0      ; restore first arg to resolved call
END(_dl_linux_resolve)
