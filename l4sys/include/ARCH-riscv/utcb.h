/**
 * \file
 * \brief   UTCB definitions for RISC-V.
 * \ingroup l4_utcb_api
 */
/* SPDX-License-Identifier: ((GPL-2.0-only WITH mif-exception) OR LicenseRef-kk-custom) */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */
#pragma once

#include <l4/sys/types.h>

/**
 * \defgroup l4_utcb_api_riscv RISC-V Virtual Registers (UTCB)
 * \ingroup  l4_utcb_api
 */

enum L4_riscv_exc_cause
{
  L4_riscv_exc_inst_misaligned        = 0,
  L4_riscv_exc_inst_access            = 1,
  L4_riscv_exc_illegal_inst           = 2,
  L4_riscv_exc_breakpoint             = 3,
  L4_riscv_exc_load_acesss            = 5,
  L4_riscv_exc_store_acesss           = 7,
  L4_riscv_exc_ecall                  = 8,
  L4_riscv_exc_inst_page_fault        = 12,
  L4_riscv_exc_load_page_fault        = 13,
  L4_riscv_exc_store_page_fault       = 15,

  L4_riscv_ec_l4_ipc_upcall           = 0x18,
  L4_riscv_ec_l4_exregs_exception     = 0x19,
  L4_riscv_ec_l4_debug_ipi            = 0x1a,
  L4_riscv_ec_l4_alien_after_syscall  = 0x1b,
};

/**
 * \brief UTCB structure for exceptions.
 * \ingroup l4_utcb_api_riscv
 */
typedef struct l4_exc_regs_t
{
  l4_umword_t eret_work;
  union
  {
    l4_umword_t r[31];
    struct
    {
      l4_umword_t ra;
      l4_umword_t sp;
      l4_umword_t gp;
      l4_umword_t tp;
      l4_umword_t t0, t1, t2;
      l4_umword_t s0, s1;
      l4_umword_t a0, a1, a2, a3, a4, a5, a6, a7;
      l4_umword_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
      l4_umword_t t3, t4, t5, t6;
    };
  };

  union { l4_umword_t pc; l4_umword_t ip; };
  l4_umword_t status;
  l4_umword_t cause;
  union { l4_umword_t tval; l4_umword_t pfa; };
} l4_exc_regs_t;

/**
 * \brief UTCB constants for RISC-V
 * \ingroup l4_utcb_api_riscv
 * \hideinitializer
 */
enum L4_utcb_consts_riscv
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
  // We store the UTCB pointer immediately before the TCB.
  l4_addr_t tp;
  __asm__ ("mv %0, tp" : "=r" (tp));
  return *(l4_utcb_t **)(tp - 3 * sizeof(void*));
}

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
  return u->cause;
}

L4_INLINE int l4_utcb_exc_is_pf(l4_exc_regs_t const *u) L4_NOTHROW
{
  switch(u->cause)
    {
    case L4_riscv_exc_inst_access:
    case L4_riscv_exc_load_acesss:
    case L4_riscv_exc_store_acesss:
    case L4_riscv_exc_inst_page_fault:
    case L4_riscv_exc_load_page_fault:
    case L4_riscv_exc_store_page_fault:
      return 1;
    default:
      return 0;
    }
}

L4_INLINE l4_addr_t l4_utcb_exc_pfa(l4_exc_regs_t const *u) L4_NOTHROW
{
  return u->tval | (u->cause == L4_riscv_exc_store_page_fault ? 2 : 0);
}

L4_INLINE int l4_utcb_exc_is_ex_regs_exception(l4_exc_regs_t const *u) L4_NOTHROW
{
  return l4_utcb_exc_typeval(u) == L4_riscv_ec_l4_exregs_exception;
}
