/*
 * Copyright (C) 2009, 2025 Technische Universität Dresden.
 * Author(s): Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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
  l4_umword_t err;
};

static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue,
                         Sig_arch_context const &, l4_umword_t pfa)
{
  ucf->uc_mcontext.fault_address = pfa;

  for (unsigned i = 0; i < 31; ++i)
    ucf->uc_mcontext.regs[i] = ue->r[i];

  ucf->uc_mcontext.sp        = ue->sp;
  ucf->uc_mcontext.pc        = ue->pc;
  ucf->uc_mcontext.pstate    = ue->pstate;
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  unsigned i;

  for (i = 0; i < 31; ++i)
    ue->r[i] = ucf->uc_mcontext.regs[i];

  ue->sp = ucf->uc_mcontext.sp;
  ue->pc = ucf->uc_mcontext.pc;
  ue->pstate = ucf->uc_mcontext.pstate;
}

static inline
void setup_sighandler_frame(l4_exc_regs_t *u, ucontext_t *ucf,
                            siginfo_t const *si, struct sigaction const &sa)
{
  u->pc = reinterpret_cast<l4_umword_t>(&sigenter);
  u->sp = reinterpret_cast<l4_addr_t>(ucf);
  u->r[0] = si->si_signo;
  u->r[1] = reinterpret_cast<l4_umword_t>(si);
  u->r[2] = reinterpret_cast<l4_umword_t>(ucf);
  u->r[16] = (sa.sa_flags & SA_SIGINFO)
             ? reinterpret_cast<l4_umword_t>(sa.sa_sigaction)
             : reinterpret_cast<l4_umword_t>(sa.sa_handler);
  u->r[19] = reinterpret_cast<l4_umword_t>(&ucf->uc_mcontext.__reserved);
}

static inline
Exc_cause map_exception_to_signal(l4_exc_regs_t const &regs, siginfo_t *si,
                                  Sig_arch_context *arch)
{
  switch (regs.err >> 26)
    {
    case 0: // Unknown
      si->si_signo = SIGILL;
      si->si_code  = ILL_ILLOPC;
      si->si_addr  = reinterpret_cast<void *>(regs.pc);
      break;
    case 0x13: // Branch Target Exception
    case 0x1c: // Exception from a PAC Fail
      si->si_signo = SIGSEGV;
      si->si_code = SEGV_ACCERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 0x22: // PC alignment fault exception
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRALN;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 0x20: // Prefetch Abort from a lower Exception level
    case 0x21: // Prefetch Abort taken without a change in Exception level
    case 0x24: // Data Abort exception from a lower Exception level
    case 0x25: // Data Abort exception taken without a change in Exception level
      switch (regs.err & 0b111111)
        {
        case 0b000100 ... 0b000111:
        case 0b101011:
          // Translation fault level x
          si->si_signo = SIGSEGV;
          si->si_code = SEGV_MAPERR;
          break;
        case 0b001000 ... 0b001111:
          // Access flag / Permission fault
          si->si_signo = SIGSEGV;
          si->si_code = SEGV_ACCERR;
          break;
        case 0b100001:
          // Alignment fault
          si->si_signo = SIGBUS;
          si->si_code = BUS_ADRALN;
          break;
        default:
          // Synchronous External abort, ECC errors and the rest..
          si->si_signo = SIGBUS;
          si->si_code = BUS_ADRERR;
          break;
        }
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 0x26: // SP alignment fault exception
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRALN;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 0x28: // Trapped floating-point exception taken from AArch32 state
    case 0x2c: // Trapped floating-point exception taken from AArch64 state
      si->si_signo = SIGFPE;
      si->si_code = 0;
      if (regs.err & (1U << 23))  // TFV - Trapped Fault Valid bit
        {
          if (regs.err & (1U << 0)) // IOF - Invalid Operation
            si->si_code = FPE_FLTINV;
          else if (regs.err & (1U << 1))  // DZF - Divide by Zero
            si->si_code = FPE_FLTDIV;
          else if (regs.err & (1U << 2))  // OFF - Overflow
            si->si_code = FPE_FLTOVF;
          else if (regs.err & (1U << 3))  // UFF - Underflow
            si->si_code = FPE_FLTUND;
          else if (regs.err & (1U << 4))  // IXF - Inexact
            si->si_code = FPE_FLTRES;
        }
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 0x2f: // SError exception
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 0x30: // Breakpoint exception from a lower Exception level
    case 0x31: // Breakpoint exception taken without a change in Exception level
    case 0x32: // Software Step exception from a lower Exception level
    case 0x33: // Software Step exception taken without a change in Exception level
    case 0x34: // Watchpoint exception from a lower Exception level
    case 0x35: // Watchpoint exception taken without a change in Exception level
    case 0x38: // BKPT instruction execution in AArch32 state
    case 0x3c: // BRK instruction execution in AArch64 state
      si->si_signo = SIGTRAP;
      si->si_code = TRAP_BRKPT; // FIXME
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 0x3e: // L4_THREAD_EX_REGS_TRIGGER_EXCEPTION
      return Exc_cause::Ex_regs;
    default:
      return Exc_cause::Unknown;
    }

  arch->err = regs.err;

  return Exc_cause::Signal;
}

static void
dump_exception_state(L4Re::Util::Err const &err, l4_exc_regs_t const *r)
{
  static char const * const modes[32] = {
    "EL0t", "",     "",     "",
    "EL1t", "EL1h", "",     "",
    "EL2t", "EL2h", "",     "",
    "EL3t", "EL3h", "",     "",
    "usr",  "fiq",  "irq",  "svc",
    "",     "",     "mon",  "abt",
    "",     "",     "",     "und",
    "",     "",     "",     "sys",
  };

  for (int i = 0; i < 30; i += 2)
    err.printf("r[%2d] = %#18lx  r[%2d] = %#18lx\n", i, r->r[i], i+1,
               r->r[i+1]);

  err.printf("r[30] = %#18lx\n", r->r[30]);
  err.printf("pc    = %#18lx  sp    = %#18lx\n", r->pc, r->sp);

  err.printf("\n");
  err.printf("pstate   = %#18lx (%c%c%c%c %s %s)\n", r->pstate,
             (r->pstate & (1UL << 31)) ? 'N' : 'n',
             (r->pstate & (1UL << 30)) ? 'Z' : 'z',
             (r->pstate & (1UL << 29)) ? 'C' : 'c',
             (r->pstate & (1UL << 28)) ? 'V' : 'v',
             (r->pstate & 0x10) ? ((r->pstate & 0x20) ? "T32" : "A32") : "A64",
             modes[r->pstate & 0x1f]);
  err.printf("err      = %#18lx  pfa      = %#18lx\n", r->err, r->pfa);
  err.printf("tpidruro = %#18lx  tpidrurw = %#18lx\n", r->tpidruro,
             r->tpidrurw);
}
