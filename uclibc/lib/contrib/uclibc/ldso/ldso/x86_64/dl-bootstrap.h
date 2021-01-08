/* vi: set sw=4 ts=4: */
/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005 by Mike Frysinger <vapier@gentoo.org>
 *
 * Parts taken from glibc/sysdeps/x86_64/dl-machine.h
 */

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, ElfW(Addr) *reloc_addr,
	ElfW(Addr) symbol_addr, ElfW(Addr) load_addr, ElfW(Sym) *sym)
{
	switch (ELF_R_TYPE(rpnt->r_info)) {
		case R_X86_64_GLOB_DAT:
		case R_X86_64_JUMP_SLOT:
			*reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		case R_X86_64_DTPMOD64:
			*reloc_addr = 1;
			break;
		case R_X86_64_NONE:
		case R_X86_64_DTPOFF64:
			break;
		case R_X86_64_TPOFF64:
			*reloc_addr = sym->st_value + rpnt->r_addend - symbol_addr;
			break;
/*TODO:		case R_X86_64_RELATIVE:
			*reloc_addr = load_addr + rpnt->r_addend;
			break; */
		/* Start: added by aw11 */
		case R_X86_64_RELATIVE:
			*reloc_addr += load_addr;
			break;
		/* End: added by aw11 */
		default:
			_dl_exit(1);
	}
}
