!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.

!    clone() is even more special than fork() as it mucks with stacks
!    and invokes a function in the right context after its all over.

#include <asm/errno.h>
#include <asm/unistd.h>

#define CLONE_VM      0x00000100
#define CLONE_THREAD  0x00010000

#ifdef __PIC__
#define __CLONE_METAG_LOAD_TP ___metag_load_tp@PLT
#else
#define __CLONE_METAG_LOAD_TP ___metag_load_tp
#endif

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
			pid_t *ptid, struct user_desc *tls, pid_t *ctid); */

	.text
	.global	__clone
	.type	__clone,function
__clone:
	! sanity check args
	MOV	D0Re0, #-EINVAL
	CMP	D1Ar1, #0
	BEQ	___error
	CMP	D0Ar2, #0
	BEQ     ___error

	! save function pointer
	MOV D0FrT, D1Ar1

	! do the system call
	MOV	D1Ar1, D1Ar3
	MOV	D1Ar3, D1Ar5
	MOV	D1Ar5, D0Ar6
	MOV	D0Ar6, D0Ar4
	GETD	D0Ar4, [A0StP+#-4]

	! new sp is already in D0Ar2
	MOV	D1Re0, #__NR_clone
	SWITCH	#0x440001
	CMP	D0Re0,#0
	! Error on -1
	BLT	___error
	! If non-zero we are the parent
	MOVNE	PC, D1RtP
	! BRKPNT

	! We are the child
#ifdef RESET_PID
	SETL	[A0StP++], D0FrT, D1RtP
	MOVT	D0FrT, #HI(CLONE_THREAD)
	ADD	D0FrT, D0FrT, #LO(CLONE_THREAD)
	ANDS	D0FrT, D0FrT, D1Ar1
	BNZ	3f
	MOVT	D0FrT, #HI(CLONE_VM)
	ADD	D0FrT, D0FrT, #LO(CLONE_VM)
	ANDS	D0FrT, D0FrT, D1Ar1
	BZ	1f
	MOV	D1Ar1, #-1
	BA	2f
1:	MOV	D1Re0, #__NR_getpid
	SWITCH	#0x440001
	MOV	D1Ar1, D0Re0
2:	CALLR	D1RtP, __CLONE_METAG_LOAD_TP
	SUB	D0Re0, D0Re0, #TLS_PRE_TCB_SIZE
	SETD	[D0Re0 + #PID], D1Ar1
	SETD	[D0Re0 + #TID], D1Ar1
3:	GETL	D0FrT, D1RtP, [--A0StP]
#endif
	! Rearrange the function arg and call address from registers
	MOV	D0Ar2, D0FrT
	MOV	D1Ar1, D0Ar6
	MOV	D1RtP, PC
	ADD	D1RtP, D1RtP, #8
	MOV	PC, D0Ar2

	! and we are done, passing the return value D0Re0 through D1Ar1
	MOV	D1Ar1, D0Re0
#ifdef __PIC__
	B	_exit@PLT
#else
	B	_exit
#endif

___error:
	MOV	D1Ar1, D0Re0
#ifdef __PIC__
	B	___syscall_error@PLT
#else
	B	___syscall_error
#endif
	.size	__clone, .-__clone

.weak    _clone
_clone    =   __clone
