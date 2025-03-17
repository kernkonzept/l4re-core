/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 */

#ifndef _ARCH_DL_SYSDEP
#define _ARCH_DL_SYSDEP

/* Define this if the system uses RELOCA.  */
#undef ELF_USES_RELOCA
#include <elf.h>

#ifdef __FDPIC__
/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

#define DL_CHECK_LIB_TYPE(epnt, piclib, _dl_progname, libname) \
do \
{ \
  (piclib) = 2; \
} \
while (0)
#endif /* __FDPIC__ */

/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE) \
{				\
  GOT_BASE[2] = (unsigned long) _dl_linux_resolve; \
  GOT_BASE[1] = (unsigned long) MODULE; \
}

static __always_inline unsigned long arm_modulus(unsigned long m, unsigned long p)
{
	unsigned long i,t,inc;
	i=p; t=0;
	while (!(i&(1<<31))) {
		i<<=1;
		t++;
	}
	t--;
	for (inc=t;inc>2;inc--) {
		i=p<<inc;
		if (i&(1<<31))
			break;
		while (m>=i) {
			m-=i;
			i<<=1;
			if (i&(1<<31))
				break;
			if (i<p)
				break;
		}
	}
	while (m>=p) {
		m-=p;
	}
	return m;
}
#define do_rem(result, n, base) ((result) = arm_modulus(n, base))
#define do_div_10(result, remain) ((result) = (((result) - (remain)) / 2) * -(-1ul / 5ul))

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_ARM
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "ARM"

struct elf_resolve;
unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.

   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */

#ifdef __FDPIC__
/* Avoid R_ARM_ABS32 to go through the PLT so that R_ARM_TARGET1
   translated to R_ARM_ABS32 doesn't use the PLT: otherwise, this
   breaks init_array because functions are referenced through the
   PLT.  */
#define elf_machine_type_class(type)					\
  ((((type) == R_ARM_JUMP_SLOT || (type) == R_ARM_TLS_DTPMOD32		\
     || (type) == R_ARM_FUNCDESC_VALUE || (type) == R_ARM_FUNCDESC || (type) == R_ARM_ABS32 \
     || (type) == R_ARM_TLS_DTPOFF32 || (type) == R_ARM_TLS_TPOFF32)	\
    * ELF_RTYPE_CLASS_PLT)						\
   | (((type) == R_ARM_COPY) * ELF_RTYPE_CLASS_COPY))
#else
#define elf_machine_type_class(type)									\
  ((((type) == R_ARM_JUMP_SLOT || (type) == R_ARM_TLS_DTPMOD32			\
     || (type) == R_ARM_TLS_DTPOFF32 || (type) == R_ARM_TLS_TPOFF32)	\
    * ELF_RTYPE_CLASS_PLT)												\
   | (((type) == R_ARM_COPY) * ELF_RTYPE_CLASS_COPY))
#endif /* __FDPIC__ */

#ifdef __FDPIC__
/* We must force strings used early in the bootstrap into the data
   segment.  */
#undef SEND_EARLY_STDERR
#define SEND_EARLY_STDERR(S) \
  do { /* FIXME: implement */; } while (0)

#undef INIT_GOT
#include "../fdpic/dl-sysdep.h"
#endif /* __FDPIC__ */

/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr __attribute__ ((unused))
elf_machine_load_address (void)
{
  extern const Elf32_Ehdr __ehdr_start attribute_hidden;
  return (Elf32_Addr) &__ehdr_start;
}

/* Return the link-time address of _DYNAMIC. */
static __always_inline Elf32_Addr __attribute__ ((unused))
elf_machine_dynamic (void)
{
  extern Elf32_Dyn _DYNAMIC[] attribute_hidden;
  return (Elf32_Addr) _DYNAMIC - elf_machine_load_address ();
}

static __always_inline void
#ifdef __FDPIC__
elf_machine_relative (DL_LOADADDR_TYPE load_off, const Elf32_Addr rel_addr,
#else
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
#endif
		      Elf32_Word relative_count)
{
#if defined(__FDPIC__)
    Elf32_Rel *rpnt = (void *) rel_addr;

    do {
        unsigned long *reloc_addr = (unsigned long *) DL_RELOC_ADDR(load_off, rpnt->r_offset);

        *reloc_addr = DL_RELOC_ADDR(load_off, *reloc_addr);
        rpnt++;
#else
    Elf32_Rel * rpnt = (void *) rel_addr;
    --rpnt;
    do {
      Elf32_Addr *const reloc_addr = (void *) (load_off + (++rpnt)->r_offset);
      *reloc_addr += load_off;
#endif
    } while(--relative_count);
}
#endif /* !_ARCH_DL_SYSDEP */

#ifdef __ARM_EABI__
#define DL_MALLOC_ALIGN 8	/* EABI needs 8 byte alignment for STRD LDRD */
#endif
