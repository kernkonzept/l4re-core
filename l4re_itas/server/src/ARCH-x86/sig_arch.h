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
  l4_umword_t trapno;
  l4_umword_t err;
};

static inline int get_es(void)
{ int r; asm volatile("mov %%es, %0" : "=r"(r)); return r; }

static inline int get_ds(void)
{ int r; asm volatile("mov %%ds, %0" : "=r"(r)); return r; }

static inline int get_cs(void)
{ int r; asm volatile("mov %%cs, %0" : "=r"(r)); return r; }

static inline int get_ss(void)
{ int r; asm volatile("mov %%ss, %0" : "=r"(r)); return r; }

static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue,
                         Sig_arch_context const &arch, l4_umword_t pfa)
{
  ucf->uc_mcontext.gregs[REG_GS]     = ue->gs;
  ucf->uc_mcontext.gregs[REG_FS]     = ue->fs;
  ucf->uc_mcontext.gregs[REG_ES]     = get_es(); // we do not have other values
  ucf->uc_mcontext.gregs[REG_DS]     = get_ds();

  ucf->uc_mcontext.gregs[REG_EDI]    = ue->edi;
  ucf->uc_mcontext.gregs[REG_ESI]    = ue->esi;
  ucf->uc_mcontext.gregs[REG_EBP]    = ue->ebp;
  ucf->uc_mcontext.gregs[REG_ESP]    = ue->sp;
  ucf->uc_mcontext.gregs[REG_EBX]    = ue->ebx;
  ucf->uc_mcontext.gregs[REG_EDX]    = ue->edx;
  ucf->uc_mcontext.gregs[REG_ECX]    = ue->ecx;
  ucf->uc_mcontext.gregs[REG_EAX]    = ue->eax;

  ucf->uc_mcontext.gregs[REG_TRAPNO] = arch.trapno;
  ucf->uc_mcontext.gregs[REG_ERR]    = arch.err;
  ucf->uc_mcontext.cr2               = pfa;

  ucf->uc_mcontext.gregs[REG_EIP]    = ue->ip;
  ucf->uc_mcontext.gregs[REG_CS]     = get_cs();

  ucf->uc_mcontext.gregs[REG_EFL]    = ue->flags;
  ucf->uc_mcontext.gregs[REG_UESP]   = ue->sp;
  ucf->uc_mcontext.gregs[REG_SS]     = get_ss();
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  ue->gs     = ucf->uc_mcontext.gregs[REG_GS];
  ue->fs     = ucf->uc_mcontext.gregs[REG_FS];

  ue->edi    = ucf->uc_mcontext.gregs[REG_EDI];
  ue->esi    = ucf->uc_mcontext.gregs[REG_ESI];
  ue->ebp    = ucf->uc_mcontext.gregs[REG_EBP];
  ue->ebx    = ucf->uc_mcontext.gregs[REG_EBX];
  ue->edx    = ucf->uc_mcontext.gregs[REG_EDX];
  ue->ecx    = ucf->uc_mcontext.gregs[REG_ECX];
  ue->eax    = ucf->uc_mcontext.gregs[REG_EAX];

  ue->sp     = ucf->uc_mcontext.gregs[REG_ESP];
  ue->ip     = ucf->uc_mcontext.gregs[REG_EIP];
  ue->flags  = ucf->uc_mcontext.gregs[REG_EFL];
}

static inline
void setup_sighandler_frame(l4_exc_regs_t *u, ucontext_t *ucf,
                            siginfo_t const *si, struct sigaction const &sa)
{
  ucf->uc_mcontext.fpregs = &ucf->__fpregs_mem;

  // Make sure there is enough space for fxsave64...
  static_assert(sizeof(ucf->__fpregs_mem) >= 464);

  // x86 also assumes a 16 byte aligned stack...
  l4_umword_t *sp = reinterpret_cast<l4_umword_t *>(ucf);
  --sp;
  *--sp = reinterpret_cast<l4_umword_t>(ucf);
  *--sp = reinterpret_cast<l4_umword_t>(si);
  *--sp = si->si_signo;

  u->ip = reinterpret_cast<l4_umword_t>(&sigenter);
  u->sp = reinterpret_cast<l4_addr_t>(sp);
  u->edi = reinterpret_cast<l4_umword_t>(&ucf->__fpregs_mem);
  u->esi = (sa.sa_flags & SA_SIGINFO)
          ? reinterpret_cast<l4_umword_t>(sa.sa_sigaction)
          : reinterpret_cast<l4_umword_t>(sa.sa_handler);
}

static inline
Exc_cause map_exception_to_signal(l4_exc_regs_t const &regs, siginfo_t *si,
                                  Sig_arch_context *arch)
{
  switch (regs.trapno)
    {
    case 0: // #DE - Divide Error
      si->si_signo = SIGFPE;
      si->si_code = FPE_INTDIV;
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 1:
      si->si_signo = SIGTRAP;
      si->si_code = TRAP_BRKPT; // FIXME
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case  2: // NMI - Non-maskable Interrupt
    case  3: // BP - Breakpoint
    case  4: // OF - Overflow
    case  5: // BR - Bound Range Exceeded
      return Exc_cause::Unknown;
    case  6: // UD - Invalid Opcode
      si->si_signo = SIGILL;
      si->si_code  = ILL_ILLOPC;
      si->si_addr  = reinterpret_cast<void *>(regs.ip);
      break;
    case  7: // NM - Device Not Available
    case  8: // DF - Double Fault
      return Exc_cause::Unknown;
    case  9: // OLD_MF - Coprocessor Segment Overrun
      si->si_signo = SIGFPE;
      break;
    case 10: // TS - Invalid TSS
      si->si_signo = SIGSEGV;
      break;
    case 11: // NP - Segment Not Present
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 12: // SS - Stack Segment Fault
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 13: // GP - General Protection Fault
      si->si_signo = SIGSEGV;
      si->si_code = SEGV_ACCERR;
      // TODO: try do decipher memory reference if applicable
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 14: // PF - Page Fault
      si->si_signo = SIGSEGV;
      si->si_code = SEGV_MAPERR;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 15: // SPURIOUS - Spurious Interrupt
      return Exc_cause::Unknown;
    case 16: // MF - x87 Floating-Point Exception
      si->si_signo = SIGFPE;
      si->si_code = 0; // TODO: fpu__exception_code()
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 17: // AC - Alignment Check
      si->si_signo = SIGBUS;
      si->si_code = BUS_ADRALN;
      si->si_addr = reinterpret_cast<void *>(regs.pfa);
      break;
    case 18: // MC - Machine Check
      return Exc_cause::Unknown;
    case 19: // XF - SIMD Floating-Point Exception
      si->si_signo = SIGFPE;
      si->si_code = 0; // TODO: fpu__exception_code()
      si->si_addr = reinterpret_cast<void *>(regs.ip);
      break;
    case 20: // VE - Virtualization Exception
    case 21: // CP - Control Protection Exception
    case 29: // VC - VMM Communication Exception
    case 32: // IRET - IRET Exception
      return Exc_cause::Unknown;
    case 0xff: // L4_THREAD_EX_REGS_TRIGGER_EXCEPTION
      return Exc_cause::Ex_regs;
    default:
      return Exc_cause::Unknown;
    }

  arch->trapno = regs.trapno;
  arch->err = regs.err;

  return Exc_cause::Signal;
}
