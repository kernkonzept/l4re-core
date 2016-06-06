/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__
#include <errno.h>

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) ((unsigned int) (val) >= 0xfffff001u)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)	(-(val))

#define X(x) #x
#define Y(x) X(x)

#define __issue_syscall(syscall_name)                   \
"       syscall  "  Y(syscall_name) ";                    \n"

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)	\
(__extension__ 						\
({							\
	register long __result __asm__("$r0");		\
	register long _sys_num __asm__("$r8");		\
							\
	LOAD_ARGS_##nr (name, args)			\
        _sys_num = (name);				\
							\
        __asm__ volatile (				\
		__issue_syscall (name)                  \
		: "=r" (__result)			\
		: "r"(_sys_num) ASM_ARGS_##nr		\
		: "$lp", "memory");			\
	__result;					\
}) \
)

/* Macros for setting up inline __asm__ input regs */
#define ASM_ARGS_0
#define ASM_ARGS_1	ASM_ARGS_0, "r" (__result)
#define ASM_ARGS_2	ASM_ARGS_1, "r" (_arg2)
#define ASM_ARGS_3	ASM_ARGS_2, "r" (_arg3)
#define ASM_ARGS_4	ASM_ARGS_3, "r" (_arg4)
#define ASM_ARGS_5	ASM_ARGS_4, "r" (_arg5)
#define ASM_ARGS_6	ASM_ARGS_5, "r" (_arg6)
#define ASM_ARGS_7	ASM_ARGS_6, "r" (_arg7)

/* Macros for converting sys-call wrapper args into sys call args */
#define LOAD_ARGS_0(name, arg)					\
	_sys_num = (long) (name);				\

#define LOAD_ARGS_1(name, arg1)					\
	__result = (long) (arg1);					\
	LOAD_ARGS_0 (name, arg1)

/*
 * Note that the use of _tmpX might look superflous, however it is needed
 * to ensure that register variables are not clobbered if arg happens to be
 * a function call itself. e.g. sched_setaffinity() calling getpid() for arg2
 *
 * Also this specific order of recursive calling is important to segregate
 * the tmp args evaluation (function call case described above) and assigment
 * of register variables
 */
#define LOAD_ARGS_2(name, arg1, arg2)				\
	long _tmp2 = (long) (arg2);				\
	LOAD_ARGS_1 (name, arg1)				\
	register long _arg2 __asm__ ("$r1") = _tmp2;

#define LOAD_ARGS_3(name, arg1, arg2, arg3)			\
	long _tmp3 = (long) (arg3);				\
	LOAD_ARGS_2 (name, arg1, arg2)				\
	register long _arg3 __asm__ ("$r2") = _tmp3;

#define LOAD_ARGS_4(name, arg1, arg2, arg3, arg4)		\
	long _tmp4 = (long) (arg4);				\
	LOAD_ARGS_3 (name, arg1, arg2, arg3)			\
	register long _arg4 __asm__ ("$r3") = _tmp4;

#define LOAD_ARGS_5(name, arg1, arg2, arg3, arg4, arg5)		\
	long _tmp5 = (long) (arg5);				\
	LOAD_ARGS_4 (name, arg1, arg2, arg3, arg4)		\
	register long _arg5 __asm__ ("$r4") = _tmp5;

#define LOAD_ARGS_6(name,  arg1, arg2, arg3, arg4, arg5, arg6)	\
	long _tmp6 = (long) (arg6);				\
	LOAD_ARGS_5 (name, arg1, arg2, arg3, arg4, arg5)	\
	register long _arg6 __asm__ ("$r5") = _tmp6;

#define LOAD_ARGS_7(name, arg1, arg2, arg3, arg4, arg5, arg6, arg7)\
	long _tmp7 = (long) (arg7);				\
	LOAD_ARGS_6 (name, arg1, arg2, arg3, arg4, arg5, arg6)	\
	register long _arg7 __asm__ ("$r6") = _tmp7;

#endif /* ! __ASSEMBLER__  */
#endif /* _BITS_SYSCALLS_H */
