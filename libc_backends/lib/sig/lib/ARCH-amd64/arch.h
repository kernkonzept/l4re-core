/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#pragma once

#include <ucontext.h>
#include <sys/ucontext.h>
#include <stdio.h>

static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue)
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

  ucf->uc_mcontext.gregs[REG_TRAPNO] = ue->trapno;
  ucf->uc_mcontext.gregs[REG_ERR]    = ue->err;
  ucf->uc_mcontext.gregs[REG_CR2]    = ue->pfa;

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

  ue->trapno = ucf->uc_mcontext.gregs[REG_TRAPNO];
  ue->err    = ucf->uc_mcontext.gregs[REG_ERR];

  ue->sp     = ucf->uc_mcontext.gregs[REG_RSP];
  ue->ip     = ucf->uc_mcontext.gregs[REG_RIP];
  ue->flags  = ucf->uc_mcontext.gregs[REG_EFL];
}

static inline
void show_regs(l4_exc_regs_t *u)
{
  printf("Exception: State:  Trap=%ld\n", u->trapno);
  printf("Want more? Code it...\n");
}
