/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <asm/errno.h>
#include <sys/syscall.h>
#include <sysdep.h>

; Per man, libc clone( ) is as follows
;
; int clone(int (*fn)(void *), void *child_stack,
;           int flags, void *arg, ...
;           /* pid_t *ptid, struct user_desc *tls, pid_t *ctid */);
;
; NOTE: I'm assuming that the last 3 args are NOT var-args and in case all
;	3 are not relevant, caller will nevertheless pass those as NULL.
;       Current (Jul 2012) upstream powerpc/clone.S assumes similarly.
;	Our LTP (from 2007) doesn't seem to have tests to prove otherwise

; clone syscall in kernel
;
; int sys_clone(unsigned long clone_flags, unsigned long newsp,
;		int __user *parent_tidptr, void *tls,
;		int __user *child_tidptr)


ENTRY(clone)
	cmp	r0, 0		; @fn can't be NULL
	cmp.ne	r1, 0		; @child_stack can't be NULL
	bz	.L__sys_err

	; @fn and @args needed after the syscall for child
	; However r3 containing @arg will be clobbered BEFORE syscall
	; r0 containg @fn will be clobbered AFTER syscall (with ret val)
	mov	r10, r0
	mov	r11, r3

	; adjust libc args for syscall
	mov 	r0, r2		; libc @flags is 1st syscall arg
	mov	r2, r4		; libc @ptid
	mov	r3, r5		; libc @tls
	mov	r4, r6		; libc @ctid
	mov	r8, __NR_clone
	ARC_TRAP_INSN

	cmp	r0, 0		; return code : 0 new process, !0 parent
	blt	.L__sys_err2	; < 0 (signed) error
	jnz	[blink]		; Parent returns

	; child jumps off to @fn with @arg as argument
	j.d	[r10]
	mov	r0, r11

	; falls thru to _exit() with result from @fn (already in r0)
	b	HIDDEN_JUMPTARGET(_exit)

.L__sys_err:
	mov	r0, -EINVAL
.L__sys_err2:
	; (1) No need to make -ve kernel error code as positive errno
	;   __syscall_error expects the -ve error code returned by kernel
	; (2) r0 still had orig -ve kernel error code
	; (3) Tail call to __syscall_error so we dont have to come back
	;     here hence instead of jmp-n-link (reg push/pop) we do jmp
	; (4) No need to route __syscall_error via PLT, B is inherently
	;     position independent
	b   __syscall_error
END(clone)
libc_hidden_def(clone)
