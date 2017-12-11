/* vi: set sw=4 ts=4: */
/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2016 by Alexander Warg <alexander.warg@kernkonzept.com>
 */

#ifndef _ARCH_DL_SYSDEP
#define _ARCH_DL_SYSDEP

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA 1
#include <elf.h>
#include <link.h>
/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)                    \
{                                                    \
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve; \
	GOT_BASE[1] = (unsigned long) MODULE;            \
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

static __always_inline unsigned long arm_modulus(unsigned long m, unsigned long p)
{
	unsigned long i,t,inc;
	i=p; t=0;
	while (!(i&(1UL<<63))) {
		i<<=1;
		t++;
	}
	t--;
	for (inc=t;inc>2;inc--) {
		i=p<<inc;
		if (i&(1UL<<63))
			break;
		while (m>=i) {
			m-=i;
			i<<=1;
			if (i&(1UL<<63))
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
#define MAGIC1 EM_AARCH64
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "aarch64"

struct elf_resolve;
unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/*
 * ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
 * TLS variable, so undefined references should not be allowed to
 * define the value.
 *
 * ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
 * of the main executable's symbols, as for a COPY reloc.
 */
#define elf_machine_type_class(type)    \
  ((((type) == R_AARCH64_JUMP_SLOT      \
     || (type) == R_AARCH64_TLS_DTPMOD  \
     || (type) == R_AARCH64_TLS_DTPREL  \
     || (type) == R_AARCH64_TLS_TPREL   \
     || (type) == R_AARCH64_TLSDESC) * ELF_RTYPE_CLASS_PLT) \
   | (((type) == R_AARCH64_COPY) * ELF_RTYPE_CLASS_COPY))
/* aw11: disable this feature for now
   | (((type) == R_AARCH64_GLOB_DAT) * ELF_RTYPE_CLASS_EXTERN_PROTECTED_DATA))
*/

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
 * first element of the GOT.
 */
static inline ElfW(Addr) __attribute__ ((unused))
elf_machine_dynamic (void)
{
	extern const ElfW(Addr) _GLOBAL_OFFSET_TABLE_[] attribute_hidden;
	return _GLOBAL_OFFSET_TABLE_[0];
}

static inline ElfW(Addr) __attribute__ ((unused))
elf_machine_load_address (void)
{
	ElfW(Addr) static_addr;
	ElfW(Addr) dynamic_addr;

	__asm__ (
			"	adrp    %1, _dl_start\n"
			"	add     %1, %1, #:lo12:_dl_start\n"
			"	ldr     %w0, 1f\n"
			"	b       2f\n"
			"1:	.word   _dl_start\n"
			"2:\n"
			: "=r" (static_addr),  "=r" (dynamic_addr));
	return dynamic_addr - static_addr;
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
