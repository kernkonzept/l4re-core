/*
 * Copyright (C) 2009, 2025 Technische Universität Dresden.
 * Author(s): Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *            Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include "../sig_arch_ifc.h"

enum : l4_addr_t
{
#ifdef __mips64
  Sig_stack_align = 16
#else
  Sig_stack_align = 8
#endif
};

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
  for (unsigned i = 0; i < 32; ++i)
    ucf->uc_mcontext.gregs[i] = ue->r[i];

  ucf->uc_mcontext.mdhi = ue->hi;
  ucf->uc_mcontext.mdlo = ue->lo;
  ucf->uc_mcontext.pc = ue->epc;
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  for (unsigned i = 0; i < 32; ++i)
    ue->r[i] = ucf->uc_mcontext.gregs[i];

  ue->hi = ucf->uc_mcontext.mdhi;
  ue->lo = ucf->uc_mcontext.mdlo;
  ue->epc = ucf->uc_mcontext.pc;
}

static inline
void setup_sighandler_frame(l4_exc_regs_t *u, ucontext_t *ucf,
                            siginfo_t const *si, struct sigaction const &sa)
{
  u->epc = reinterpret_cast<l4_umword_t>(&sigenter);
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
  if ((regs.cause & 0x1ff) == 0x101) // L4_THREAD_EX_REGS_TRIGGER_EXCEPTION
    return Exc_cause::Ex_regs;

  switch ((regs.cause) >> 2 & 0x1f)
    {
    case 2: // TLB Invalid - instruction fetch or load
    case 3: // TLB Invalid - store
      si->si_signo = SIGSEGV;
      si->si_code = SEGV_MAPERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 4: // Address alignment error exception (load or insn fetch)
    case 5: // Address alignment error exceptin (store)
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRALN;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 6: // Bus error exception (insn)
    case 7: // Bus error exception (load/store)
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 9: // BREAK instruction
      si->si_signo = SIGTRAP;
      si->si_code = TRAP_BRKPT;
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 10: // Reserved instruction exception
      si->si_signo = SIGILL;
      si->si_code = ILL_ILLOPC;
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 11: // Coprocessor not enabled
      si->si_signo = SIGILL;
      si->si_code = ILL_COPROC;
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 12: // Arithmetic overflow
      si->si_signo = SIGFPE;
      si->si_code = FPE_INTOVF;
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 13: // Trap
      si->si_signo = SIGTRAP;
      si->si_code = TRAP_BRKPT;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 15: // Floating-Point exception
      si->si_signo = SIGFPE;
      si->si_code = 0;
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 20: // TLB Execute Inhibit.
      si->si_signo = SIGSEGV;
      si->si_code = SEGV_ACCERR;
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 23: // Watch instruction/data fetch/load/store
      si->si_signo = SIGTRAP;
      si->si_code = TRAP_BRKPT;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    default:
      return Exc_cause::Unknown;
    }

  return Exc_cause::Signal;
}

static void
dump_exception_state(L4Re::Util::Err const &err, l4_exc_regs_t const *r)
{
  for (int i = 0; i < 32; i += 2)
    err.printf("r[%2d]  = %#18lx  r[%2d] = %#18lx\n", i, r->r[i], i+1,
               r->r[i+1]);
  err.printf("\n");
  err.printf("hi     = %#18lx  lo    = %#18lx\n", r->hi, r->lo);
  err.printf("status = %#18lx\n", r->status);
  err.printf("ip     = %#18lx  ulr   = %#18lx\n", r->ip, r->ulr);
  err.printf("cause  = %#18lx  pfa   = %#18lx\n", r->cause, r->pfa);
  err.printf("bad_instr_p = %#18lx\n", r->bad_instr_p);
  err.printf("bad_instr   = %#18lx\n", r->bad_instr);
}
