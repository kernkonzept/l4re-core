/*!
 * \file
 * \brief  Inter-thread setjmp/longjmp
 *
 * \date   12/21/2005
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
#ifndef __UTIL_INCLUDE_ARCH_AMD64_L4API_L4F_SETJMP_H_
#define __UTIL_INCLUDE_ARCH_AMd64_L4API_L4F_SETJMP_H_
#include <l4/sys/types.h>
#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

typedef struct{
    l4_umword_t r8;		/* 0x00 */
    l4_umword_t r9;		/* 0x08 */
    l4_umword_t r10;		/* 0x10 */
    l4_umword_t r11;		/* 0x18 */
    l4_umword_t r12;		/* 0x20 */
    l4_umword_t r13;		/* 0x28 */
    l4_umword_t r14;		/* 0x30 */
    l4_umword_t r15;		/* 0x38 */
    l4_umword_t rbx;		/* 0x40 */
    l4_umword_t rsi;		/* 0x48 */
    l4_umword_t rbp;		/* 0x50 */
    l4_umword_t rsp;		/* 0x58 */
    l4_umword_t rip;		/* 0x60 */
    l4_umword_t rip_caller;	/* 0x68 */
    l4_umword_t rflags;		/* 0x70 */
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
L4_CV int l4_thread_setjmp(l4_thread_jmp_buf env);

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
L4_CV void l4_thread_longjmp(l4_threadid_t thread, l4_thread_jmp_buf env, int val);

EXTERN_C_END

#endif
