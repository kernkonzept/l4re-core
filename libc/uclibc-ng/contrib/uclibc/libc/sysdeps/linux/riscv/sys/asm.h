/* Miscellaneous macros.
   Copyright (C) 2000-2017 Free Software Foundation, Inc.
   Copyright (C) 2021 Kernkonzept GmbH.

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

#ifndef _SYS_ASM_H
#define _SYS_ASM_H

#ifndef __ASSEMBLER__
# define __RISCV_ASM_INST(inst) #inst
#else
# define __RISCV_ASM_INST(inst) inst
#endif

/*
 * Macros to handle different pointer/register sizes for 32/64-bit code
 */
#if __riscv_xlen == 64
# define PTRLOG 3
# define SZREG __RISCV_ASM_INST(8)
# define SZMOD __RISCV_ASM_INST(d)
# define REG_S __RISCV_ASM_INST(sd)
# define REG_L __RISCV_ASM_INST(ld)
#elif __riscv_xlen == 32
# define PTRLOG 2
# define SZREG __RISCV_ASM_INST(4)
# define SZMOD __RISCV_ASM_INST(w)
# define REG_S __RISCV_ASM_INST(sw)
# define REG_L __RISCV_ASM_INST(lw)
#else
# error __riscv_xlen must equal 32 or 64
#endif

#ifdef __riscv_flen
# if __riscv_flen == 32
#  define FREG_L __RISCV_ASM_INST(flw)
#  define FREG_S __RISCV_ASM_INST(fsw)
#  define SZFREG __RISCV_ASM_INST(4)
# elif __riscv_flen == 64
#  define FREG_L __RISCV_ASM_INST(fld)
#  define FREG_S __RISCV_ASM_INST(fsd)
#  define SZFREG __RISCV_ASM_INST(8)
# else
#  error unsupported FLEN
# endif
#endif

#define ENTRY(symbol)     \
  .globl symbol;          \
  .align 2;               \
  .type symbol,@function; \
symbol:

#undef END
#define END(function) \
  .size function,.-function

/* Stack alignment */
#define ALMASK ~15

#endif /* sys/asm.h */
