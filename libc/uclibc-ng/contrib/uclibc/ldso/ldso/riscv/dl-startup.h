/* Copyright (C) 2011-2021 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <sys/asm.h>

/* Partly adapted from glibc/sysdeps/riscv/dl-machine.h */

#define STRINGXP(X) __STRING(X)

/* Initial entry point code for the dynamic linker.
   The C function `_dl_start' is the real entry point;
   its return value is the user program's entry point. */

__asm__(
  ".text \n"
  " .globl  _start \n"
  " .type   _start, @function \n"
  " .hidden _start \n"
  "_start: \n"
  " mv a0, sp \n"
  " jal _dl_start \n"
  // Stash user entry point in s0.
  " mv s0, a0 \n"
  // See if we were run as a command with the executable file
  // name as an extra leading argument.
  " lw a0, _dl_skip_args \n"
  // Load the original argument count.
  " " REG_L " a1, 0(sp) \n"
  // Subtract _dl_skip_args from it.
  " sub a1, a1, a0 \n"
  // Adjust the stack pointer to skip _dl_skip_args words.
  " sll a0, a0, " STRINGXP(PTRLOG) "\n"
  " add sp, sp, a0 \n"
  // Save back the modified argument count.
  " " REG_S " a1, 0(sp) \n"
  // Pass our finalizer function to _start.
  " lla a0, _dl_fini \n"
  // Jump to the user entry point.
  " jr s0 \n"
  ".size _start , . -_start \n"
  " .previous"
);

/* Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*)ARGS)+1)

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
  unsigned long symbol_addr, unsigned long load_addr, ElfW(Sym) *symtab)
{
  (void) symtab;

  switch (ELF_R_TYPE(rpnt->r_info)) {
    case R_RISCV_NONE:
      break;
    case R_RISCV_RELATIVE:
      *reloc_addr = load_addr + rpnt->r_addend;
      break;
    case R_RISCV_JUMP_SLOT:
    case __WORDSIZE == 64 ? R_RISCV_64 : R_RISCV_32:
      *reloc_addr = symbol_addr + rpnt->r_addend;
       break;
    case R_RISCV_COPY:
      /* break; */
    default:
      SEND_STDERR("Unsupported relocation type\n");
      _dl_exit(1);
  }
}
