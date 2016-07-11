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
  unsigned i;

  ucf->uc_mcontext.fault_address = ue->pfa;

  for (i = 0; i < 31; ++i)
    ucf->uc_mcontext.regs[i] = ue->r[i];

  ucf->uc_mcontext.sp        = ue->sp;
  ucf->uc_mcontext.pc        = ue->pc;
  ucf->uc_mcontext.pstate    = ue->pstate;
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  unsigned i;

  ue->pfa = ucf->uc_mcontext.fault_address;

  for (i = 0; i < 31; ++i)
    ue->r[i] = ucf->uc_mcontext.regs[i];

  ue->sp = ucf->uc_mcontext.sp;
  ue->pc = ucf->uc_mcontext.pc;
  ue->pstate = ucf->uc_mcontext.pstate;
}

static inline
void show_regs(l4_exc_regs_t *u)
{
  printf("Exception: State:\n");
  printf(" r0=%08lx  r1=%08lx  r2=%08lx  r3=%08lx\n r4=%08lx  r5=%08lx  r6=%08lx  r7=%08lx\n",
         u->r[0], u->r[1], u->r[2], u->r[3], u->r[4], u->r[5], u->r[6], u->r[7]);
  printf(" r8=%08lx  r9=%08lx r10=%08lx r11=%08lx\nr12=%08lx  sp=%08lx  pc=%08lx\n",
         u->r[8], u->r[9], u->r[10], u->r[11], u->r[12], u->sp, u->pc);
  printf("pstate=%08lx err=%08lx pfa=%08lx\n", u->pstate, u->err, u->pfa);
}
