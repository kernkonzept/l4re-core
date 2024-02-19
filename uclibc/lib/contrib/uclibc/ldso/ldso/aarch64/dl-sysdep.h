/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2016 by Alexander Warg <alexander.warg@kernkonzept.com>
 * Copyright (C) 2017-2018 by Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Ported from GNU C Library
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 1995-2017 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA

#include <elf.h>
#include <link.h>

/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE) \
{				\
  GOT_BASE[2] = (unsigned long) _dl_linux_resolve; \
  GOT_BASE[1] = (unsigned long) MODULE; \
}

#define ARCH_NUM 2
#define DT_TLSDESC_PLT_IDX  (DT_NUM + OS_NUM)
#define DT_TLSDESC_GOT_IDX  (DT_NUM + OS_NUM + 1)

#define ARCH_DYNAMIC_INFO(dpnt,  dynamic, debug_addr)   \
do {                                                    \
	if (dpnt->d_tag == DT_TLSDESC_PLT)                  \
		dynamic[DT_TLSDESC_PLT_IDX] = dpnt->d_un.d_val; \
	else if (dpnt->d_tag == DT_TLSDESC_GOT)             \
		dynamic[DT_TLSDESC_GOT_IDX] = dpnt->d_un.d_val; \
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_AARCH64
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "aarch64"

struct elf_resolve;
unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

#define elf_machine_type_class(type)					\
  ((((type) == R_AARCH64_JUMP_SLOT 					\
     || (type) == R_AARCH64_TLS_DTPMOD					\
     || (type) == R_AARCH64_TLS_DTPREL 					\
     || (type) == R_AARCH64_TLS_TPREL					\
     || (type) == R_AARCH64_TLSDESC) * ELF_RTYPE_CLASS_PLT)		\
   | (((type) == R_AARCH64_COPY) * ELF_RTYPE_CLASS_COPY))
/* aw11: disable this feature for now
   | (((type) == R_AARCH64_GLOB_DAT) * ELF_RTYPE_CLASS_EXTERN_PROTECTED_DATA))
*/

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT. */
extern const ElfW(Addr) _GLOBAL_OFFSET_TABLE_[] attribute_hidden;
static __always_inline ElfW(Addr) __attribute__ ((unused))
elf_machine_dynamic (void)
{
  return _GLOBAL_OFFSET_TABLE_[0];
}

/* Return the run-time load address of the shared object.  */

static __always_inline ElfW(Addr) __attribute__ ((unused))
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
elf_machine_relative(Elf64_Addr load_off, const Elf64_Addr rel_addr,
                     Elf64_Word relative_count)
{
	Elf64_Rela *rpnt = (Elf64_Rela*)rel_addr;
	--rpnt;
	do {
		Elf64_Addr *const reloc_addr = (Elf64_Addr*)(load_off + (++rpnt)->r_offset);

		*reloc_addr = load_off + rpnt->r_addend;
	} while (--relative_count);
}
