/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>
#include <sys/procfs.h>

typedef int greg_t;

/* Number of general registers.  */
#define NGREG	32

/* Container for all general registers.  */
typedef elf_gregset_t gregset_t;

/* Number of each register is the `gregset_t' array.  */
enum
{
  R0 = 0,
#define R0	R0
  R1 = 1,
#define R1	R1
  R2 = 2,
#define R2	R2
  R3 = 3,
#define R3	R3
  R4 = 4,
#define R4	R4
  R5 = 5,
#define R5	R5
  R6 = 6,
#define R6	R6
  R7 = 7,
#define R7	R7
  R8 = 8,
#define R8	R8
  R9 = 9,
#define R9	R9
  R10 = 10,
#define R10	R10
  R11 = 11,
#define R11	R11
  R12 = 12,
#define R12	R12
  R13 = 13,
#define R13	R13
  R14 = 14,
#define R14	R14
  R15 = 15,
#define R15	R15
  R16 = 16,
#define R16     R16
  R17 = 17,
#define R17     R17
  R18 = 18,
#define R18     R18
  R19 = 19,
#define R19     R19
  R20 = 20,
#define R20     R20
  R21 = 21,
#define R21     R21
  R22 = 22,
#define R22     R22
  R23 = 23,
#define R23     R23
  R24 = 24,
#define R24     R24
  R25 = 25,
#define R25     R25
  R26 = 26,
#define R26     R26
  R27 = 27,
#define R27     R27
  R28 = 28,
#define R28     R28
  R29 = 29,
#define R29     R29
  R30 = 30,
#define R30     R30
  R31 = 31
#define R31     R31

};

/* Structure to describe FPU registers.  */
typedef elf_fpregset_t	fpregset_t;

/* Context to describe whole processor state.  */
typedef struct
  {
    gregset_t gregs;
    fpregset_t fpregs;
  } mcontext_t;

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    __sigset_t uc_sigmask;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    long int uc_filler[5];
  } ucontext_t;

#endif /* sys/ucontext.h */
