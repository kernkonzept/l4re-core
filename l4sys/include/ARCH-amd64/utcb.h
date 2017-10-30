/**
 * \file
 * \brief   UTCB definitions for amd64.
 * \ingroup l4_utcb_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
/*****************************************************************************/
#ifndef __L4_SYS__INCLUDE__ARCH_AMD64__UTCB_H__
#define __L4_SYS__INCLUDE__ARCH_AMD64__UTCB_H__

#include <l4/sys/types.h>

/**
 * \defgroup l4_utcb_api_amd64 amd64 Virtual Registers (UTCB)
 * \ingroup  l4_utcb_api
 */

/**
 * \brief UTCB constants for AMD64
 * \ingroup l4_utcb_api_amd64
 */
enum L4_utcb_consts_amd64
{
  L4_UTCB_EXCEPTION_REGS_SIZE    = 26,
  L4_UTCB_GENERIC_DATA_SIZE      = 63,
  L4_UTCB_GENERIC_BUFFERS_SIZE   = 58,

  L4_UTCB_MSG_REGS_OFFSET        = 0,
  L4_UTCB_BUF_REGS_OFFSET        = 64 * sizeof(l4_umword_t),
  L4_UTCB_THREAD_REGS_OFFSET     = 123 * sizeof(l4_umword_t),

  L4_UTCB_INHERIT_FPU            = 1UL << 24,
  L4_UTCB_OFFSET                 = 1024,
};

/**
 * \brief UTCB structure for exceptions.
 * \ingroup l4_utcb_api_amd64
 */
typedef struct l4_exc_regs_t
{
  l4_umword_t r15;        /**< r15 */
  l4_umword_t r14;        /**< r14 */
  l4_umword_t r13;        /**< r13 */
  l4_umword_t r12;        /**< r12 */
  l4_umword_t r11;        /**< r11 */
  l4_umword_t r10;        /**< r10 */
  l4_umword_t r9;         /**< r9 */
  l4_umword_t r8;         /**< r8 */
  l4_umword_t rdi;        /**< rdi */
  l4_umword_t rsi;        /**< rsi */
  l4_umword_t rbp;        /**< rbp */
  l4_umword_t pfa;        /**< page fault address */
  l4_umword_t rbx;        /**< rbx */
  l4_umword_t rdx;        /**< rdx */
  l4_umword_t rcx;        /**< rcx */
  l4_umword_t rax;        /**< rax */

  l4_umword_t trapno;     /**< trap number */
  l4_umword_t err;        /**< error code */
  l4_umword_t ip;         /**< instruction pointer */
  l4_umword_t dummy1;
  l4_umword_t flags;      /**< rflags */
  l4_umword_t sp;         /**< stack pointer */
  l4_umword_t ss;         /**< stack segment register */
  l4_umword_t fs_base;
  l4_umword_t gs_base;
  l4_uint16_t ds, es, fs, gs;
} l4_exc_regs_t;


#include_next <l4/sys/utcb.h>

/*
 * ==================================================================
 * Implementations.
 */

L4_INLINE l4_utcb_t *l4_utcb_direct(void) L4_NOTHROW
{
  l4_utcb_t *res;
  __asm__ ( "mov %%gs:0, %0 \n" : "=r"(res));
  return res;
}

L4_INLINE l4_umword_t l4_utcb_exc_pc(l4_exc_regs_t const *u) L4_NOTHROW
{
  return u->ip;
}

L4_INLINE void l4_utcb_exc_pc_set(l4_exc_regs_t *u, l4_addr_t pc) L4_NOTHROW
{
  u->ip = pc;
}

L4_INLINE l4_umword_t l4_utcb_exc_typeval(l4_exc_regs_t const *u) L4_NOTHROW
{
  return u->trapno;
}

L4_INLINE int l4_utcb_exc_is_pf(l4_exc_regs_t const *u) L4_NOTHROW
{
  return u->trapno == 14;
}

L4_INLINE l4_addr_t l4_utcb_exc_pfa(l4_exc_regs_t const *u) L4_NOTHROW
{
  return (u->pfa & ~7UL) | (u->err & 2);
}

L4_INLINE int l4_utcb_exc_is_ex_regs_exception(l4_exc_regs_t const *u) L4_NOTHROW
{
  return l4_utcb_exc_typeval(u) == 0xff;
}

#endif /* ! __L4_SYS__INCLUDE__ARCH_AMD64__UTCB_H__ */
