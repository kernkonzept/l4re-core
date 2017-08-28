/**
 * \file
 * \brief   UTCB definitions for ARM64.
 * \ingroup  l4_utcb_api
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
#ifndef __L4_SYS__INCLUDE__ARCH_ARM64__UTCB_H__
#define __L4_SYS__INCLUDE__ARCH_ARM64__UTCB_H__

#include <l4/sys/types.h>

/**
 * \defgroup l4_utcb_api_arm64 ARM64 Virtual Registers (UTCB)
 * \ingroup  l4_utcb_api
 */

/**
 * \brief UTCB structure for exceptions.
 * \ingroup l4_utcb_api_arm64
 */
typedef struct l4_exc_regs_t
{
  l4_umword_t eret_work;
  l4_umword_t r[31];
  l4_umword_t reserved;
  l4_umword_t err;

  l4_umword_t pfa;
  l4_umword_t sp;
  union { l4_umword_t ip; l4_umword_t pc; }; /* aliases for PC */
  union { l4_umword_t flags; l4_umword_t pstate; }; /* aliases for PSTATE (PSR) */
  l4_umword_t tpidruro;
} l4_exc_regs_t;

/**
 * \brief UTCB constants for ARM64
 * \ingroup l4_utcb_api_arm64
 * \hideinitializer
 */
enum L4_utcb_consts_arm64
{
  L4_UTCB_EXCEPTION_REGS_SIZE    = sizeof(l4_exc_regs_t) / sizeof(l4_umword_t),
  L4_UTCB_GENERIC_DATA_SIZE      = 63,
  L4_UTCB_GENERIC_BUFFERS_SIZE   = 58,

  L4_UTCB_MSG_REGS_OFFSET        = 0,
  L4_UTCB_BUF_REGS_OFFSET        = 64 * sizeof(l4_umword_t),
  L4_UTCB_THREAD_REGS_OFFSET     = 123 * sizeof(l4_umword_t),

  L4_UTCB_INHERIT_FPU            = 1UL << 24,

  L4_UTCB_OFFSET                 = 1024,
};

#include_next <l4/sys/utcb.h>

/*
 * ==================================================================
 * Implementations.
 */

#ifdef __GNUC__
L4_INLINE l4_utcb_t *l4_utcb_direct(void) L4_NOTHROW
{
  l4_utcb_t *utcb;
  __asm__ ("mrs %0, TPIDRRO_EL0" : "=r" (utcb));
  return utcb;
}
#endif

L4_INLINE l4_umword_t l4_utcb_exc_pc(l4_exc_regs_t const *u) L4_NOTHROW
{
  return u->pc;
}

L4_INLINE void l4_utcb_exc_pc_set(l4_exc_regs_t *u, l4_addr_t pc) L4_NOTHROW
{
  u->pc = pc;
}

L4_INLINE l4_umword_t l4_utcb_exc_typeval(l4_exc_regs_t const *u) L4_NOTHROW
{
  return u->err >> 26;
}

L4_INLINE int l4_utcb_exc_is_pf(l4_exc_regs_t const *u) L4_NOTHROW
{
  return ((u->err >> 26) & 0x30) == 0x20;
}

L4_INLINE l4_addr_t l4_utcb_exc_pfa(l4_exc_regs_t const *u) L4_NOTHROW
{
  return (u->pfa & ~7UL) | ((u->err >> 5) & 2);
}

L4_INLINE int l4_utcb_exc_is_ex_regs_exception(l4_exc_regs_t const *u) L4_NOTHROW
{
  return (u->err >> 26) == 0x3e;
}

#endif /* ! __L4_SYS__INCLUDE__ARCH_ARM64__UTCB_H__ */
