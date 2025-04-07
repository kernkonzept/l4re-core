/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 */

/* Define this if the system uses RELOCA.  */
#undef ELF_USES_RELOCA
#include <elf.h>
/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)							\
do {														\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve;		\
	GOT_BASE[1] = (unsigned long) MODULE;					\
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_386
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "386"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_386_JMP_SLOT || (type) == R_386_TLS_DTPMOD32		      \
     || (type) == R_386_TLS_DTPOFF32 || (type) == R_386_TLS_TPOFF32	      \
     || (type) == R_386_TLS_TPOFF) * ELF_RTYPE_CLASS_PLT)				  \
   | (((type) == R_386_COPY) * ELF_RTYPE_CLASS_COPY))


/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr attribute_unused
elf_machine_load_address (void)
{
	extern const Elf32_Ehdr __ehdr_start attribute_hidden;
	return (Elf32_Addr) &__ehdr_start;
}

/* Return the link-time address of _DYNAMIC. */
extern const Elf32_Addr _GLOBAL_OFFSET_TABLE_[] attribute_hidden;
static __always_inline Elf32_Addr __attribute__ ((unused, const))
elf_machine_dynamic (void)
{
	extern Elf32_Dyn _DYNAMIC[] attribute_hidden;
	return (Elf32_Addr) _DYNAMIC - elf_machine_load_address ();
}

static __always_inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	Elf32_Rel * rpnt = (void *) rel_addr;
	--rpnt;
	do {
		Elf32_Addr *const reloc_addr = (void *) (load_off + (++rpnt)->r_offset);

		*reloc_addr += load_off;
	} while (--relative_count);
}
