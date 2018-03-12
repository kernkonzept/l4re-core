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
  ucf->uc_mcontext.trap_no       = 0;
  ucf->uc_mcontext.error_code    = ue->err;
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
  ucf->uc_mcontext.fault_address = ue->pfa;
}

static inline
void fill_utcb_exc(l4_exc_regs_t *ue, ucontext_t *ucf)
{
  ue->err  = ucf->uc_mcontext.error_code;

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
  ue->pfa  = ucf->uc_mcontext.fault_address;
}

static inline
void show_regs(l4_exc_regs_t *u)
{
  printf("Exception: State:\n");
  printf(" r0=%08lx  r1=%08lx  r2=%08lx  r3=%08lx\n r4=%08lx  r5=%08lx  r6=%08lx  r7=%08lx\n",
         u->r[0], u->r[1], u->r[2], u->r[3], u->r[4], u->r[5], u->r[6], u->r[7]);
  printf(" r8=%08lx  r9=%08lx r10=%08lx r11=%08lx\nr12=%08lx  sp=%08lx  lr=%08lx  pc=%08lx\n",
         u->r[8], u->r[9], u->r[10], u->r[11], u->r[12], u->sp, u->ulr, u->pc);
  printf("psr=%08lx err=%08lx pfa=%08lx\n", u->cpsr, u->err, u->pfa);
}
