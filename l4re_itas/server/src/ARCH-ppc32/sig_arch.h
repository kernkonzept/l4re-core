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
};

static inline
void fill_ucontext_frame(ucontext_t *ucf, l4_exc_regs_t *ue,
                         Sig_arch_context const &, l4_umword_t pfa)
{
  ucf->uc_mcontext.regs->trap          = 0;

  ucf->uc_mcontext.regs->gpr[0]        = ue->r[0];
  // ...
  ucf->uc_mcontext.regs->dar           = pfa;
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  ue->r[0] = ucf->uc_mcontext.regs->gpr[0];
  // ...
}

static inline
void setup_sighandler_frame(l4_exc_regs_t *, ucontext_t *,
                            siginfo_t const *, struct sigaction const &)
{
  // ...
}

static inline
Exc_cause map_exception_to_signal(l4_exc_regs_t const &, siginfo_t *,
                                  Sig_arch_context *)
{
  // ...
  return Exc_cause::Unknown;
}

static void
dump_exception_state(L4Re::Util::Err const &, l4_exc_regs_t const *)
{
  // ...
}
