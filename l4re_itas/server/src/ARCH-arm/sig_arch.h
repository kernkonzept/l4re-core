/*
 * Copyright (C) 2009, 2025 Technische Universität Dresden.
 * Author(s): Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *            Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include "../sig_arch_ifc.h"

enum : l4_addr_t { Sig_stack_align = 8 };

static inline
l4_addr_t sig_adjust_stack(l4_addr_t sp)
{ return sp & ~(Sig_stack_align - 1U); }

struct Sig_arch_context
{
  l4_umword_t err;
};

static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue,
                         Sig_arch_context const &arch, l4_umword_t pfa)
{
  ucf->uc_mcontext.trap_no       = 0;
  ucf->uc_mcontext.error_code    = arch.err;
  ucf->uc_mcontext.oldmask       = 0;

  ucf->uc_mcontext.arm_r0        = ue->r[0];
  ucf->uc_mcontext.arm_r1        = ue->r[1];
  ucf->uc_mcontext.arm_r2        = ue->r[2];
  ucf->uc_mcontext.arm_r3        = ue->r[3];
  ucf->uc_mcontext.arm_r4        = ue->r[4];
  ucf->uc_mcontext.arm_r5        = ue->r[5];
  ucf->uc_mcontext.arm_r6        = ue->r[6];
  ucf->uc_mcontext.arm_r7        = ue->r[7];
  ucf->uc_mcontext.arm_r8        = ue->r[8];
  ucf->uc_mcontext.arm_r9        = ue->r[9];
  ucf->uc_mcontext.arm_r10       = ue->r[10];
  ucf->uc_mcontext.arm_fp        = ue->r[11];
  ucf->uc_mcontext.arm_ip        = ue->r[12];
  ucf->uc_mcontext.arm_sp        = ue->sp;
  ucf->uc_mcontext.arm_lr        = ue->ulr;
  ucf->uc_mcontext.arm_pc        = ue->pc;
  ucf->uc_mcontext.arm_cpsr      = ue->cpsr;
  ucf->uc_mcontext.fault_address = pfa;
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  ue->r[0] = ucf->uc_mcontext.arm_r0;
  ue->r[1] = ucf->uc_mcontext.arm_r1;
  ue->r[2] = ucf->uc_mcontext.arm_r2;
  ue->r[3] = ucf->uc_mcontext.arm_r3;
  ue->r[4] = ucf->uc_mcontext.arm_r4;
  ue->r[5] = ucf->uc_mcontext.arm_r5;
  ue->r[6] = ucf->uc_mcontext.arm_r6;
  ue->r[7] = ucf->uc_mcontext.arm_r7;
  ue->r[8] = ucf->uc_mcontext.arm_r8;
  ue->r[9] = ucf->uc_mcontext.arm_r9;
  ue->r[10]= ucf->uc_mcontext.arm_r10;
  ue->r[11]= ucf->uc_mcontext.arm_fp;
  ue->r[12]= ucf->uc_mcontext.arm_ip;
  ue->sp   = ucf->uc_mcontext.arm_sp;
  ue->ulr  = ucf->uc_mcontext.arm_lr;
  ue->pc   = ucf->uc_mcontext.arm_pc;
  ue->cpsr = ucf->uc_mcontext.arm_cpsr;
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
  u->r[12] = (sa.sa_flags & SA_SIGINFO)
             ? reinterpret_cast<l4_umword_t>(sa.sa_sigaction)
             : reinterpret_cast<l4_umword_t>(sa.sa_handler);
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
    case 0x3e: // L4_THREAD_EX_REGS_TRIGGER_EXCEPTION
      return Exc_cause::Ex_regs;
    default:
      return Exc_cause::Unknown;
    }

  arch->err = regs.err;

  return Exc_cause::Signal;
}
