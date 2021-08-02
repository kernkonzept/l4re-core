/* SPDX-License-Identifier: LGPL-2.1-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */
#pragma once

#include <l4/util/l4_macros.h>

#include <ucontext.h>
#include <sys/ucontext.h>
#include <stdio.h>

static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue)
{
  unsigned i;

  ucf->uc_mcontext.gregs[0] = ue->pc;

  for (i = 0; i < 31; ++i)
    ucf->uc_mcontext.gregs[i+1] = ue->r[i];
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  unsigned i;

  ue->pc = ucf->uc_mcontext.gregs[0];

  for (i = 0; i < 31; ++i)
    ue->r[i] = ucf->uc_mcontext.gregs[i+1];
}

static inline
void show_regs(l4_exc_regs_t *u)
{
  printf("Exception: State:\n");
  printf("pc     = " l4_addr_fmt " sp    = " l4_addr_fmt " ra  = " l4_addr_fmt "\n",
         u->pc, u->sp, u->ra);
  printf("status = " l4_addr_fmt " cause = " l4_addr_fmt " pfa = " l4_addr_fmt "\n",
         u->status, u->cause, u->pfa);
}
