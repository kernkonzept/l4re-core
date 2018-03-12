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

static inline int get_es(void)
{ int r; asm volatile("mov %%es, %0" : "=r"(r)); return r; }

static inline int get_ds(void)
{ int r; asm volatile("mov %%ds, %0" : "=r"(r)); return r; }

static inline int get_cs(void)
{ int r; asm volatile("mov %%cs, %0" : "=r"(r)); return r; }

static inline int get_ss(void)
{ int r; asm volatile("mov %%ss, %0" : "=r"(r)); return r; }

static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue)
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

  ucf->uc_mcontext.gregs[REG_TRAPNO] = ue->trapno;
  ucf->uc_mcontext.gregs[REG_ERR]    = ue->err;
  ucf->uc_mcontext.cr2               = ue->pfa;

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

  ue->trapno = ucf->uc_mcontext.gregs[REG_TRAPNO];
  ue->err    = ucf->uc_mcontext.gregs[REG_ERR];

  ue->sp     = ucf->uc_mcontext.gregs[REG_ESP];
  ue->ip     = ucf->uc_mcontext.gregs[REG_EIP];
  ue->flags  = ucf->uc_mcontext.gregs[REG_EFL];
}

static inline
void show_regs(l4_exc_regs_t *u)
{
  printf("Exception: State:  Trap=%ld\n", u->trapno);
  printf("EAX=%08lx EBX=%08lx ECX=%08lx EDX=%08lx\n", u->eax, u->ebx, u->ecx, u->edx);
  printf("ESI=%08lx EDI=%08lx EBP=%08lx          \n", u->esi, u->edi, u->ebp);
  printf("EIP=%08lx ESP=%08lx         Flags=%08lx\n", u->ip, u->sp, u->flags);
  printf(" GS=%08lx  FS=%08lx\n", u->gs, u->fs);
}
