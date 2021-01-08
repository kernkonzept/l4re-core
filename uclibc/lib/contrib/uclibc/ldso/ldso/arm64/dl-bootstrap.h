/* vi: set sw=4 ts=4: */
/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2016 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, ElfW(Sym) *symtab)
{
	(void) symtab;

	switch (ELF_R_TYPE(rpnt->r_info)) {
		case R_AARCH64_NONE:
			break;
		case R_AARCH64_RELATIVE:
			*reloc_addr = load_addr + rpnt->r_addend;
			break;
		case R_AARCH64_GLOB_DAT:
		case R_AARCH64_JUMP_SLOT:
		case R_AARCH64_ABS32:
		case R_AARCH64_ABS64:
			/*
			 * Workaround for weak undefined references which need
			 * to stay NULL
			 */
			if (symtab && symtab->st_value == 0)
				*reloc_addr = 0;
			else
				*reloc_addr = symbol_addr + rpnt->r_addend;
			 break;
#ifdef IN_RELOC_STATIC_PIE
			case R_AARCH64_TLS_DTPREL:
			case R_AARCH64_TLS_TPREL:
				/* Offsets are not affected by load address */
				break;
#endif
		case R_AARCH64_COPY:
			/* break; */
		default:
			SEND_STDERR("Unsupported relocation type\n");
			_dl_exit(1);
	}
}
