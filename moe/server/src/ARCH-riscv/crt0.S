/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */

/**
 * \file
 * \brief Startup code
 */

#include <sys/asm.h>

#if __riscv_xlen == 32
  #define ELF_HDR_E_PHOFF 0x1c
  #define ELF_HDR_E_PHNUM 0x2c
#else
  #define ELF_HDR_E_PHOFF 0x20
  #define ELF_HDR_E_PHNUM 0x38
#endif

.text

/*
 * Provide the following stack layout to uclibc:
 *
 *  0x00  0     (argc)
 *  0x08  NULL  (argv final NULL)
 *  0x10  NULL  (env final NULL)
 *              auxv[]
 *  0x18  3       AT_PHDR
 *  0x20  ?       <ptr to PHDRs>
 *  0x28  5       AT_PHNUM
 *  0x30  ?       <number of PHDRs>
 *  0x38  NULL    (auxv termination)
 *
 * This is required for C++ exceptions to work in relocated (PIE) executables
 * and with libunwind (LLVM)!
 *
 * ATTENTION: the following code assumes that the executable starts directly
 * with the ELF header! For regularly loaded executables the aux vector is
 * provided by libloader, which can work with any elf file.
 */
  .globl _real_start
_real_start:

.option push
.option norelax
  la gp, __global_pointer$
.option pop

  la sp, _stack_top
  /* Mind the argc/argv/env/auxv that needs to be on the stack! */
  addi sp, sp,  -8 * SZREG

  la t0, __executable_start

  li t1, 3                      // AT_PHDR
  REG_S t1, 3 * SZREG(sp)
  REG_L t1, ELF_HDR_E_PHOFF(t0) // e_phoff
  add t1, t0, t1
  REG_S t1, 4 * SZREG(sp)

  li t1, 5                      // AT_PHDR
  REG_S t1, 5 * SZREG(sp)
  lhu t1, ELF_HDR_E_PHNUM(t0)   // e_phoff
  REG_S t1, 6 * SZREG(sp)

  j _start

  .bss
  .balign 16
_stack_bottom:
  .space 0x4000
_stack_top:
