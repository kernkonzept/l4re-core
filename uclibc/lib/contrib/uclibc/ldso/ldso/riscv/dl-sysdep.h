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

/* Partly adapted from glibc/sysdeps/riscv/dl-machine.h */

#ifndef _ARCH_DL_SYSDEP
#define _ARCH_DL_SYSDEP

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA 1
#include <elf.h>
#include <link.h>

/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)                                 \
{                                                                 \
  GOT_BASE[0] = (unsigned long) &_dl_runtime_resolve;             \
  GOT_BASE[1] = (unsigned long) MODULE;                           \
}

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_RISCV
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "riscv"

struct elf_resolve;
extern ElfW(Addr) _dl_runtime_resolver(struct elf_resolve * tpnt, int reloc_entry);

#define elf_machine_type_class(type)        \
  ((ELF_RTYPE_CLASS_PLT * ((type) == R_RISCV_JUMP_SLOT \
     || (__WORDSIZE == 32 && (type) == R_RISCV_TLS_DTPREL32)  \
     || (__WORDSIZE == 32 && (type) == R_RISCV_TLS_DTPMOD32)  \
     || (__WORDSIZE == 32 && (type) == R_RISCV_TLS_TPREL32) \
     || (__WORDSIZE == 64 && (type) == R_RISCV_TLS_DTPREL64)  \
     || (__WORDSIZE == 64 && (type) == R_RISCV_TLS_DTPMOD64)  \
     || (__WORDSIZE == 64 && (type) == R_RISCV_TLS_TPREL64))) \
   | (ELF_RTYPE_CLASS_COPY * ((type) == R_RISCV_COPY)))

/* JMPREL relocs are inside the DT_RELA table.  */
#define ELF_MACHINE_PLTREL_OVERLAP 1

/* Return the link-time address of _DYNAMIC.  */
static inline ElfW(Addr) attribute_unused
elf_machine_dynamic (void)
{
  extern ElfW(Addr) _GLOBAL_OFFSET_TABLE_ attribute_hidden;
  return _GLOBAL_OFFSET_TABLE_;
}

static inline ElfW(Addr) attribute_unused
elf_machine_load_address (void)
{
    /* To figure out the load address we use the definition that for any symbol:
       dynamic_addr(symbol) = static_addr(symbol) + load_addr
       _DYNAMIC sysmbol is used here as its link-time address stored in
       the special unrelocated first GOT entry.  */

    extern ElfW(Dyn) _DYNAMIC[] attribute_hidden;
    return (ElfW(Addr)) &_DYNAMIC - elf_machine_dynamic ();
}

static __always_inline void
elf_machine_relative (ElfW(Addr) load_off, const ElfW(Addr) rel_addr,
                      ElfW(Word) relative_count)
{
   ElfW(Rela) * rpnt = (void *) rel_addr;
  --rpnt;
  do {
    ElfW(Addr) *const reloc_addr = (void *) (load_off + (++rpnt)->r_offset);
    *reloc_addr = load_off + rpnt->r_addend;
  } while (--relative_count);
}

#define DL_MALLOC_ALIGN 32
#endif /* !_ARCH_DL_SYSDEP */
