/*!
 * \file   l4util/lib/src/setjmp.c
 * \brief  inter-thread setjmp/longjmp
 *
 * \date   11/26/2004
 * \author Jork Loeser <jork.loeser@inf.tu-dresden.de>
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
    "l4_thread_setjmp:			\n\t"
    ".global l4_thread_setjmp		\n\t"
    "movl	4(%esp), %eax		\n\t"	/* the jump buffer */
    "movl	%ebx, (%eax)		\n\t"
    "movl	%esi, 4(%eax)		\n\t"
    "movl	%edi, 8(%eax)		\n\t"
    "movl	%ebp, 12(%eax)		\n\t"
    "movl	%eax, 16(%eax)		\n\t"	/* esp */
#ifdef __PIC__
    "call	0f			\n\t"
    "0:					\n\t"
    "addl	$(1f-0b),(%esp)		\n\t"
#else
    "lea	1f,%ecx			\n\t"
#endif
    "movl	%ecx, 20(%eax)		\n\t"	/* eip */
    "movl 	(%esp), %ecx		\n\t"
    "movl	%ecx, 24(%eax)		\n\t"	/* eip caller */
    "pushf				\n\t"
    "popl	%ecx			\n\t"
    "movl	%ecx, 28(%eax)		\n\t"	/* eflags */
    "xorl	%eax,%eax		\n\t"
    "ret				\n\t"
    /* return from longjmp. ptr to jmp_buf is on intermediate stack.
     * retval is on intermediate stack
     * esp must be restored. */
    "1:					\n\t"
    "popl	%eax			\n\t"	/* return value */
    "popl	%edx			\n\t"	/* jmp buf */
    "movl	(%edx), %ebx		\n\t"
    "movl	4(%edx), %esi		\n\t"
    "movl	8(%edx), %edi		\n\t"
    "movl	12(%edx), %ebp		\n\t"
    "movl	16(%edx), %esp		\n\t"
    "movl	28(%edx), %ecx		\n\t"
    "push	%ecx			\n\t"
    "popf				\n\t"
    "jmp  *24(%edx)			\n\t"
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
      
    l4_thread_ex_regs(thread, buf->s.eip, (l4_umword_t)stack,
		      &preempter, &pager,
                      &dummy, &dummy, &dummy);
}
