/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sysdep.h>

/* No legacy syscall ABI means NR_vfork is not available at all, use clone */
#define _SIGNAL_H
#include <bits/signum.h>       /* For SIGCHLD */

#define CLONE_VM		0x00000100
#define CLONE_VFORK		0x00004000
#define CLONE_FLAGS_FOR_VFORK	(CLONE_VM|CLONE_VFORK|SIGCHLD)

ENTRY(__vfork)
#ifdef SAVE_PID
	THREAD_SELF r1		; Get to struct pthread (just before TCB)
	ld	r2, [r1, PTHREAD_PID]
	neg.f	r3, r2
	bset.z	r3, r3, 31
	st	r3, [r1, PTHREAD_PID]
#endif
	mov	r0, CLONE_FLAGS_FOR_VFORK
	mov_s	r1, sp
	mov	r8, __NR_clone
	ARC_TRAP_INSN

	cmp	r0, 0
#ifdef RESTORE_PID
	bz	1f	; child continues
	THREAD_SELF r1		; Get to struct pthread (just before TCB)
	st	r2, [r1, PTHREAD_PID]
1:
#endif
	jge	[blink]	; pid >=0 return, else detour via tailcall to errno

	b   __syscall_error
END(__vfork)

weak_alias(__vfork,vfork)
libc_hidden_def(vfork)
