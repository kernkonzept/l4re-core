/*!
 * \file   l4util/lib/src/ARCH-amd64/setjmp.c
 * \brief  inter-thread setjmp/longjmp
 *
 * \date   12/21/2005
 * \author Jork Loeser <jork.loeser@inf.tu-dresden.de>
 * \author Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/util/setjmp.h>

int l4_thread_setjmp(l4_thread_jmp_buf env);
__asm__ (
    /* rdi points to jmp buffer */
    "mov	%r8,  0x00(%rdi)	\n\t"
    "mov	%r9,  0x08(%rdi)	\n\t"
    "mov	%r10, 0x10(%rdi)	\n\t"
    "mov	%r11, 0x18(%rdi)	\n\t"
    "mov	%r12, 0x20(%rdi)	\n\t"
    "mov	%r13, 0x28(%rdi)	\n\t"
    "mov	%r14, 0x30(%rdi)	\n\t"
    "mov	%r15, 0x38(%rdi)	\n\t"
    "mov	%rbx, 0x40(%rdi)	\n\t"
    "mov	%rsi, 0x48(%rdi)	\n\t"
    "mov	%rbp, 0x50(%rdi)	\n\t"
    "mov	%rdi, 0x58(%rdi)	\n\t"	/* rsp */
    "movabs	$1f, %rcx		\n\t"
    "mov	%rcx, 0x60(%rdi)	\n\t"	/* rip */
    "mov	(%rsp), %rcx		\n\t"
    "mov	%rcx, 0x68(%rdi)	\n\t"	/* rip caller */
    "pushf				\n\t"
    "pop	%rcx			\n\t"
    "mov	%rcx, 0x70(%rax)	\n\t"	/* rflags */
    "xor	%rax,%rax		\n\t"
"ret				\n\t"
/* return from longjmp. ptr to jmp_buf is on intermediate stack.
 * retval is on intermediate stack
 * rsp must be restored. */
"1:"
"pop	%rax			\n\t"	/* return value */
"pop	%rdi			\n\t"	/* rdi points to jmp buf */
"mov	0x00(%rdi), %r8		\n\t"
"mov	0x08(%rdi), %r9		\n\t"
"mov	0x10(%rdi), %r10	\n\t"
"mov	0x18(%rdi), %r11	\n\t"
"mov	0x20(%rdi), %r12	\n\t"
"mov	0x28(%rdi), %r13	\n\t"
"mov	0x30(%rdi), %r14	\n\t"
"mov	0x38(%rdi), %r15	\n\t"
"mov	0x40(%rdi), %rbx	\n\t"
"mov	0x48(%rdi), %rsi	\n\t"
"mov	0x50(%rdi), %rbp	\n\t"
"mov	0x58(%rdi), %rsp	\n\t"	/* rsp */
"mov	0x70(%rdi), %rcx	\n\t"	/* rflags */
"push	%rcx			\n\t"
"popf				\n\t"
"jmp	*0x68(%rdi)		\n\t"	/* rip caller */
);

void l4_thread_longjmp(l4_threadid_t thread, l4_thread_jmp_buf env,
		       int val){
    l4_thread_jmp_buf_u* buf = (l4_thread_jmp_buf_u*)env;
    l4_threadid_t preempter = L4_INVALID_ID, pager=L4_INVALID_ID;
    l4_umword_t *stack = (l4_umword_t*)((void*)buf->s.stack +
					sizeof(buf->s.stack));
    l4_umword_t dummy;

    *--stack=(l4_umword_t)env;       // the buffer
    *--stack=val?val:1;  	     // the ret-value
      
    l4_thread_ex_regs(thread, buf->s.rip, (l4_umword_t)stack,
		      &preempter, &pager,
                      &dummy, &dummy, &dummy);
}
