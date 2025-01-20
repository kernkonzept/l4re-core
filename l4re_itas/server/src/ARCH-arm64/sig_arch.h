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
    case 0x20: // Prefetch Abort from a lower Exception level
    case 0x21: // Prefetch Abort taken without a change in Exception level
      si->si_signo = SIGSEGV;
      si->si_code = SEGV_ACCERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 0x22: // PC alignment fault exception
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRALN;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 0x24: // Data Abort exception from a lower Exception level
    case 0x25: // Data Abort exception taken without a change in Exception level
      si->si_signo = SIGSEGV;
      si->si_code = SEGV_ACCERR;
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
      si->si_code = 0; // TODO: decode ISS
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
