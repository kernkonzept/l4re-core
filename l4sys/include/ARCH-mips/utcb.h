/**
 * \file
 * \brief   UTCB definitions for MIPS.
 * \ingroup l4_utcb_api
 */
#pragma once

#include <l4/sys/types.h>

/**
 * \defgroup l4_utcb_api_mips MIPS Virtual Registers (UTCB)
 * \ingroup  l4_utcb_api
 */

/**
 * \brief UTCB structure for exceptions.
 * \ingroup l4_utcb_api_mips
 *
 * l4_exc_regs_t matches l4_vcpu_regs_t and corresponds to
 * fiasco/src/kern/mips32/trap_state.cpp: Trap_state_regs and
 * entry_frame-mips32.cpp: Syscall_frame and Return_frame
 */
typedef struct l4_exc_regs_t
{
  l4_umword_t bad_instr_p;
  l4_umword_t bad_instr;
  union
  {
    l4_umword_t r[32];   /**< registers */
    struct
    {
      l4_umword_t zero;
      l4_umword_t at;
      l4_umword_t v0, v1;
      l4_umword_t a0, a1, a2, a3;
      l4_umword_t t0, t1, t2, t3, t4, t5, t6, t7;
      l4_umword_t s0, s1, s2, s3, s4, s5, s6, s7;
      l4_umword_t t8, t9;
      l4_umword_t k0, k1;
      l4_umword_t gp;
      l4_umword_t sp;
      l4_umword_t s8;
      l4_umword_t ra;
    };
  };
  l4_umword_t hi, lo;
  union
  {
    l4_umword_t pfa;
    l4_umword_t bad_v_addr;
  };
  l4_umword_t cause;
  l4_umword_t status;
  union
  {
    l4_umword_t epc;
    l4_umword_t ip;
  };
  l4_umword_t ulr;
} l4_exc_regs_t;

/**
 * \brief UTCB constants for MIPS
 * \ingroup l4_utcb_api_mips
 * \hideinitializer
 */
enum L4_utcb_consts_mips
{
  L4_UTCB_EXCEPTION_REGS_SIZE    = sizeof(l4_exc_regs_t) / sizeof(l4_umword_t),
  L4_UTCB_GENERIC_DATA_SIZE      = 63,
  L4_UTCB_GENERIC_BUFFERS_SIZE   = 58,

  L4_UTCB_MSG_REGS_OFFSET        = 0,
  L4_UTCB_BUF_REGS_OFFSET        = 64 * sizeof(l4_umword_t),
  L4_UTCB_THREAD_REGS_OFFSET     = 123 * sizeof(l4_umword_t),

  L4_UTCB_INHERIT_FPU            = 1UL << 24,

  L4_UTCB_OFFSET                 = 128 * sizeof(l4_umword_t),
};

#include_next <l4/sys/utcb.h>

/*
 * ==================================================================
 * Implementations.
 */

L4_INLINE l4_utcb_t *l4_utcb_direct(void) L4_NOTHROW
{
  /* We store the UTCB pointer at offset -0x700c from the ULR value.  This is
   * compatible with MIPS TLS used by L4Re. We provide a slot in the TCB for
   * the UTCB pointer and the TCB is stored at this offset per TLS convention.
   * The kernel also initially sets the ULR to point 0x700c behind the UTCB
   * pointer in the UTCB itself.
   */
  char const *utcb;
  __asm__ ("rdhwr %0, $29" : "=r" (utcb));
  return *(l4_utcb_t * const*)(utcb - 0x7000 - (3 * sizeof(void*)));
}

L4_INLINE l4_umword_t l4_utcb_exc_pc(l4_exc_regs_t const *u) L4_NOTHROW
{
  return u->epc;
}

L4_INLINE void l4_utcb_exc_pc_set(l4_exc_regs_t *u, l4_addr_t pc) L4_NOTHROW
{
  u->epc = pc;
}

L4_INLINE l4_umword_t l4_utcb_exc_typeval(l4_exc_regs_t const *u) L4_NOTHROW
{
  return u->cause & 0x1ff;
}

L4_INLINE int l4_utcb_exc_is_pf(l4_exc_regs_t const *u) L4_NOTHROW
{
  unsigned c = (u->cause >> 2) & 0x1f;
  return ((1 <= c) && (c <= 7)) || ((19 <= c) && (c <= 20));
}

L4_INLINE l4_addr_t l4_utcb_exc_pfa(l4_exc_regs_t const *u) L4_NOTHROW
{
  return (u->bad_v_addr & ~7) | ((u->cause >> 1) & 2);
}

L4_INLINE int l4_utcb_exc_is_ex_regs_exception(l4_exc_regs_t const *u) L4_NOTHROW
{
  return l4_utcb_exc_typeval(u) == 0x101;
}
