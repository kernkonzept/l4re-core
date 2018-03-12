/*!
 * \file
 * \brief  Inter-thread setjmp/longjmp
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
#ifndef __UTIL_INCLUDE_ARCH_X86_L4API_L4F_SETJMP_H_
#define __UTIL_INCLUDE_ARCH_X86_L4API_L4F_SETJMP_H_
#include <l4/sys/types.h>
#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

typedef struct{
    l4_umword_t ebx;		/* 0 */
    l4_umword_t esi;		/* 4 */
    l4_umword_t edi;		/* 8 */
    l4_umword_t ebp;		/* 12 */
    l4_umword_t esp;		/* 16 */
    l4_umword_t eip;		/* 20 */
    l4_umword_t eip_caller;	/* 24 */
    l4_umword_t eflags;		/* 28 */
    l4_umword_t stack[40];
} l4_thread_jmp_buf_s;
typedef int l4_thread_jmp_buf[sizeof(l4_thread_jmp_buf_s)/sizeof(l4_umword_t)];

typedef union{
    l4_thread_jmp_buf_s s;
    l4_thread_jmp_buf raw;
} l4_thread_jmp_buf_u;

/**
 * inter-thread setjmp
 *
 * Use this function to prepare a longjmp from another thread for this thread.
 *
 * \param 	env	jump buffer
 * \retval 	0	returned directly
 * \retval	!0	returned from longjmp
 *
 * \see setjmp(3)
 */
extern int l4_thread_setjmp(l4_thread_jmp_buf env);

/**
 * inter-thread longjmp
 *
 * This function sets `thread` to the location obtained by its former
 * l4_thread_setjump on `env`.
 *
 * \param	thread	thread to apply the longjmp to
 * \param	env	jump buffer
 * \param	val	0: setjmp returns with 1
 * \param	val	!0: return value of setjmp
 *
 * \see  longjmp(3)
 * \note In contrast to longjmp(3), this function returns.
 */
void l4_thread_longjmp(l4_threadid_t thread, l4_thread_jmp_buf env, int val);

EXTERN_C_END

#endif
