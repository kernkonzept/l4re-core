/* Any assembly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.
 * Copyright (C) 2005 by Joakim Tjernlund
 * Copyright (C) 2005 by Erik Andersen
 */


#include <sgidefs.h>

/*
 * Here is a macro to perform the GOT relocation. This is only
 * used when bootstrapping the dynamic loader.
 */
#define PERFORM_BOOTSTRAP_GOT(tpnt)						\
do {										\
	ElfW(Sym) *sym;								\
	ElfW(Addr) i;								\
	register ElfW(Addr) gp __asm__ ("$28");					\
	ElfW(Addr) *mipsgot = elf_mips_got_from_gpreg (gp);			\
										\
	/* Add load address displacement to all local GOT entries */		\
	i = 2;									\
	while (i < tpnt->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX])			\
		mipsgot[i++] += tpnt->loadaddr;					\
										\
	/* Handle global GOT entries */						\
	mipsgot += tpnt->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX];			\
	sym = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB] +			\
			tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];			\
	i = tpnt->dynamic_info[DT_MIPS_SYMTABNO_IDX] - tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];\
										\
	while (i--) {								\
		if (sym->st_shndx == SHN_UNDEF ||				\
			sym->st_shndx == SHN_COMMON)				\
			*mipsgot = tpnt->loadaddr + sym->st_value;		\
		else if (ELF_ST_TYPE(sym->st_info) == STT_FUNC &&		\
			*mipsgot != sym->st_value)				\
			*mipsgot += tpnt->loadaddr;				\
		else if (ELF_ST_TYPE(sym->st_info) == STT_SECTION) {		\
			if (sym->st_other == 0)					\
				*mipsgot += tpnt->loadaddr;			\
		}								\
		else								\
			*mipsgot = tpnt->loadaddr + sym->st_value;		\
										\
		mipsgot++;							\
		sym++;								\
	}									\
} while (0)

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.
 */
#if _MIPS_SIM == _MIPS_SIM_ABI64	/* consult with glibc sysdeps/mips/dl-machine.h 1.69 */
#define R_MIPS_BOOTSTRAP_RELOC ((R_MIPS_64 << 8) | R_MIPS_REL32)
#else	/* N32 || O32 */
#define R_MIPS_BOOTSTRAP_RELOC R_MIPS_REL32
#endif
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB)			\
	switch(ELF_R_TYPE((RELP)->r_info)) {					\
	case R_MIPS_BOOTSTRAP_RELOC:						\
		if (SYMTAB) {							\
			if (symtab_index<tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX])\
				*REL += SYMBOL;					\
		}								\
		else {								\
			*REL += LOAD;						\
		}								\
		break;								\
	case R_MIPS_NONE:							\
		break;								\
	default:								\
		SEND_STDERR("Aiieeee!");					\
		_dl_exit(1);							\
	}
