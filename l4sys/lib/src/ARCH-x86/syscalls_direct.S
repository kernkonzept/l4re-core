/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

__l4sys_direct_syscalls = 0xeacff000
.hidden __l4sys_direct_syscalls

	.macro	direct_syscall syscall,offset
	\syscall = __l4sys_direct_syscalls + \offset
	.global \syscall
	.hidden \syscall
	.endm


	direct_syscall	__l4sys_invoke_direct,    0x000
