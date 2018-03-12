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
void show_regs(l4_exc_regs_t *u)
{
  printf("Exception: State (tbd):\n");
  (void)u;
}
