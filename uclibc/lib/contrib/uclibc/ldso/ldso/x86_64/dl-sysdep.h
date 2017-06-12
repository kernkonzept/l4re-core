/* yoinked from glibc/sysdeps/x86_64/dl-machine.h */
/* Machine-dependent ELF dynamic relocation inline functions.  x86-64 version.
   Copyright (C) 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

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
#define INIT_GOT(GOT_BASE,MODULE)							\
do {														\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve;		\
	GOT_BASE[1] = (unsigned long) MODULE;					\
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_X86_64
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "x86_64"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type)					      \
  ((((type) == R_X86_64_JUMP_SLOT					      \
     || (type) == R_X86_64_DTPMOD64					      \
     || (type) == R_X86_64_DTPOFF64					      \
     || (type) == R_X86_64_TPOFF64)					      \
    * ELF_RTYPE_CLASS_PLT)						      \
   | (((type) == R_X86_64_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __always_inline Elf64_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
  /* This produces an IP-relative reloc which is resolved at link time. */
  extern const ElfW(Addr) _GLOBAL_OFFSET_TABLE_[] attribute_hidden;
  return _GLOBAL_OFFSET_TABLE_[0];
}


/* Return the run-time load address of the shared object.  */
static __always_inline Elf64_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
  /* Compute the difference between the runtime address of _DYNAMIC as seen
     by an IP-relative reference, and the link-time address found in the
     special unrelocated first GOT entry.  */
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
