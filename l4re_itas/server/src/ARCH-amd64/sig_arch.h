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
{
  // We have to skip over the red zone!
  return (sp - 128U) & ~(Sig_stack_align - 1U);
}

struct Sig_arch_context
{
  l4_umword_t trapno;
  l4_umword_t err;
};

static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue,
                         Sig_arch_context const &arch, l4_umword_t pfa)
{
  ucf->uc_mcontext.gregs[REG_R15]    = ue->r15;
  ucf->uc_mcontext.gregs[REG_R14]    = ue->r14;
  ucf->uc_mcontext.gregs[REG_R13]    = ue->r13;
  ucf->uc_mcontext.gregs[REG_R12]    = ue->r12;
  ucf->uc_mcontext.gregs[REG_R11]    = ue->r11;
  ucf->uc_mcontext.gregs[REG_R10]    = ue->r10;
  ucf->uc_mcontext.gregs[REG_R9]     = ue->r9;
  ucf->uc_mcontext.gregs[REG_R8]     = ue->r8;
  ucf->uc_mcontext.gregs[REG_RDI]    = ue->rdi;
  ucf->uc_mcontext.gregs[REG_RSI]    = ue->rsi;
  ucf->uc_mcontext.gregs[REG_RBP]    = ue->rbp;
  ucf->uc_mcontext.gregs[REG_RBX]    = ue->rbx;
  ucf->uc_mcontext.gregs[REG_RDX]    = ue->rdx;
  ucf->uc_mcontext.gregs[REG_RCX]    = ue->rcx;
  ucf->uc_mcontext.gregs[REG_RAX]    = ue->rax;

  ucf->uc_mcontext.gregs[REG_TRAPNO] = arch.trapno;
  ucf->uc_mcontext.gregs[REG_ERR]    = arch.err;
  ucf->uc_mcontext.gregs[REG_CR2]    = pfa;

  ucf->uc_mcontext.gregs[REG_RIP]    = ue->ip;
  ucf->uc_mcontext.gregs[REG_EFL]    = ue->flags;
  ucf->uc_mcontext.gregs[REG_RSP]    = ue->sp;
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  ue->r15    = ucf->uc_mcontext.gregs[REG_R15];
  ue->r14    = ucf->uc_mcontext.gregs[REG_R14];
  ue->r13    = ucf->uc_mcontext.gregs[REG_R13];
  ue->r12    = ucf->uc_mcontext.gregs[REG_R12];
  ue->r11    = ucf->uc_mcontext.gregs[REG_R11];
  ue->r10    = ucf->uc_mcontext.gregs[REG_R10];
  ue->r9     = ucf->uc_mcontext.gregs[REG_R9];
  ue->r8     = ucf->uc_mcontext.gregs[REG_R8];
  ue->rdi    = ucf->uc_mcontext.gregs[REG_RDI];
  ue->rsi    = ucf->uc_mcontext.gregs[REG_RSI];
  ue->rbp    = ucf->uc_mcontext.gregs[REG_RBP];
  ue->rbx    = ucf->uc_mcontext.gregs[REG_RBX];
  ue->rdx    = ucf->uc_mcontext.gregs[REG_RDX];
  ue->rcx    = ucf->uc_mcontext.gregs[REG_RCX];
  ue->rax    = ucf->uc_mcontext.gregs[REG_RAX];

  ue->sp     = ucf->uc_mcontext.gregs[REG_RSP];
  ue->ip     = ucf->uc_mcontext.gregs[REG_RIP];
  ue->flags  = ucf->uc_mcontext.gregs[REG_EFL];
}

static inline
void setup_sighandler_frame(l4_exc_regs_t *u, ucontext_t *ucf,
                            siginfo_t const *si, struct sigaction const &sa)
{
  ucf->uc_mcontext.fpregs = &ucf->__fpregs_mem;

  // Make sure there is enough space for fxsave64...
  static_assert(sizeof(ucf->__fpregs_mem) >= 464);

  u->ip = reinterpret_cast<l4_umword_t>(&sigenter);
  u->sp = reinterpret_cast<l4_addr_t>(ucf);
  u->rdi = si->si_signo;
  u->rsi = reinterpret_cast<l4_umword_t>(si);
  u->rdx = reinterpret_cast<l4_umword_t>(ucf);
  u->rcx = reinterpret_cast<l4_umword_t>(&ucf->__fpregs_mem);
  u->r8 = (sa.sa_flags & SA_SIGINFO)
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

static void
dump_exception_state(L4Re::Util::Err const &err, l4_exc_regs_t const *r)
{
  err.printf("rax = %#18lx  rbx = %#18lx\n", r->rax, r->rbx);
  err.printf("rcx = %#18lx  rdx = %#18lx\n", r->rcx, r->rdx);
  err.printf("rsi = %#18lx  rdi = %#18lx\n", r->rsi, r->rdi);
  err.printf("r8  = %#18lx  r9  = %#18lx\n", r->r8,  r->r9);
  err.printf("r10 = %#18lx  r11 = %#18lx\n", r->r10, r->r11);
  err.printf("r12 = %#18lx  r13 = %#18lx\n", r->r12, r->r13);
  err.printf("r14 = %#18lx  r15 = %#18lx\n", r->r14, r->r15);
  err.printf("rsp = %#18lx  rbp = %#18lx\n", r->sp,  r->rbp);
  err.printf("\n");
  err.printf("rip = %#18lx  rflags = %#15lx\n", r->ip,  r->flags);
  err.printf("pfa = %#18lx\n", r->pfa);
  err.printf("trap = %ld / err = %#lx\n", r->trapno, r->err);
  err.printf("\n");
  err.printf("ds = %#4x   es = %#4x   fs = %#4x   gs = %#4x   ss = %#4lx\n",
             r->ds, r->es, r->fs, r->gs, r->ss);
  err.printf("fs_base = %#14lx  gs_base = %#14lx\n", r->fs_base, r->gs_base);
}
