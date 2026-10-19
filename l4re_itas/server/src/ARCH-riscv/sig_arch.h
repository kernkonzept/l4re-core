/*
 * Copyright (C) 2021, 2024-2025 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *            Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include "../sig_arch_ifc.h"

enum : l4_addr_t { Sig_stack_align = 16 };

static inline
l4_addr_t sig_adjust_stack(l4_addr_t sp)
{ return sp & ~(Sig_stack_align - 1U); }

struct Sig_arch_context
{
};

static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue,
                         Sig_arch_context const &, l4_umword_t)
{
  unsigned i;

  ucf->uc_mcontext.__gregs[0] = ue->pc;

  for (i = 0; i < 31; ++i)
    ucf->uc_mcontext.__gregs[i+1] = ue->r[i];
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  unsigned i;

  ue->pc = ucf->uc_mcontext.__gregs[0];

  for (i = 0; i < 31; ++i)
    ue->r[i] = ucf->uc_mcontext.__gregs[i+1];
}

static inline
void setup_sighandler_frame(l4_exc_regs_t *u, ucontext_t *ucf,
                            siginfo_t const *si, struct sigaction const &sa)
{
  u->pc = reinterpret_cast<l4_umword_t>(&sigenter);
  u->sp = reinterpret_cast<l4_addr_t>(ucf);
  u->a0 = si->si_signo;
  u->a1 = reinterpret_cast<l4_umword_t>(si);
  u->a2 = reinterpret_cast<l4_umword_t>(ucf);
  u->a3 = (sa.sa_flags & SA_SIGINFO)
             ? reinterpret_cast<l4_umword_t>(sa.sa_sigaction)
             : reinterpret_cast<l4_umword_t>(sa.sa_handler);
}

static inline
Exc_cause map_exception_to_signal(l4_exc_regs_t const &regs, siginfo_t *si,
                                  Sig_arch_context *)
{
  switch (regs.cause)
    {
    case L4_riscv_exc_inst_misaligned:
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRALN;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case L4_riscv_exc_inst_access:
    case L4_riscv_exc_load_access:
    case L4_riscv_exc_store_access:
      si->si_signo = SIGSEGV;
      si->si_code = SEGV_ACCERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case L4_riscv_exc_illegal_inst:
      si->si_signo = SIGILL;
      si->si_code  = ILL_ILLOPC;
      si->si_addr  = reinterpret_cast<void *>(regs.pc);
      break;
    case L4_riscv_exc_breakpoint:
      si->si_signo = SIGTRAP;
      si->si_code = TRAP_BRKPT;
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case L4_riscv_exc_inst_page_fault:
    case L4_riscv_exc_load_page_fault:
    case L4_riscv_exc_store_page_fault:
      si->si_signo = SIGSEGV;
      si->si_code = SEGV_MAPERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case L4_riscv_ec_l4_exregs_exception:
      return Exc_cause::Ex_regs;
    default:
      return Exc_cause::Unknown;
    }

  return Exc_cause::Signal;
}

static void
dump_exception_state(L4Re::Util::Err const &err, l4_exc_regs_t const *r)
{
  static constexpr char const *regnames[30] =
  {
    "ra", "sp", "gp", "tp",
    "t0", "t1", "t2",
    "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5" /*,"t6"*/
  };
  for (int i = 0; i < 30; i += 2)
    err.printf("%3s/%sx%d = %#18lx  %3s/%sx%d = %#18lx\n",
               regnames[i],   i < 9 ? " " : "", i+1, r->r[i],
               regnames[i+1], i < 8 ? " " : "", i+2, r->r[i+1]);
  err.printf(" t6/x31 = %#18lx       pc = %#18lx\n", r->r[30], r->pc);
  err.printf("\n");
  err.printf("status  = %#18lx\n", r->status);
  err.printf("cause   = %#18lx  pfa   = %#18lx\n", r->cause, r->pfa);
  err.printf("hstatus = %#18lx\n", r->hstatus);
}
