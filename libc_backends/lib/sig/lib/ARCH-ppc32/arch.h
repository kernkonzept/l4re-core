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
  ucf->uc_mcontext.regs->trap          = 0;

  ucf->uc_mcontext.regs->gpr[0]        = ue->r[0];
  // ...
  ucf->uc_mcontext.regs->dar           = ue->pfa;
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  ue->r[0] = ucf->uc_mcontext.regs->gpr[0];
  // ...
}

static inline
void show_regs(l4_exc_regs_t *u)
{
  printf("Exception: State (tbd):\n");
  (void)u;
}
