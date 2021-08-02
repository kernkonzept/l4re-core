/* struct ucontext definition, RISC-V version.
   Copyright (C) 1997-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* Don't rely on this, the interface is currently messed up and may need to
   be broken to be fixed.  */
#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>
#include <bits/sigcontext.h>

#define NGREG	32

#define REG_PC 0
#define REG_RA 1
#define REG_SP 2
#define REG_TP 4
#define REG_S0 8
#define REG_A0 10
#define REG_NARGS 8

typedef unsigned long greg_t;

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

/* Container for floating-point state.  */
typedef union __riscv_fp_state fpregset_t;

/* Context to describe whole processor state.  */
typedef struct sigcontext mcontext_t;

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
  } ucontext_t;

#endif /* sys/ucontext.h */
