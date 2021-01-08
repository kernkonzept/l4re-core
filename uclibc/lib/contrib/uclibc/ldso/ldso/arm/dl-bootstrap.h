/* vi: set sw=4 ts=4: */
/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, Elf32_Sym *symtab)
{
	(void) symtab;

	switch (ELF_R_TYPE(rpnt->r_info)) {
		case R_ARM_NONE:
			break;
		case R_ARM_ABS32:
			*reloc_addr += symbol_addr;
			break;
		case R_ARM_PC24:
			{
				unsigned long addend;
				long newvalue, topbits;

				addend = *reloc_addr & 0x00ffffff;
				if (addend & 0x00800000) addend |= 0xff000000;

				newvalue = symbol_addr - (unsigned long)reloc_addr + (addend << 2);
				topbits = newvalue & 0xfe000000;
				if (topbits != 0xfe000000 && topbits != 0x00000000)
				{
#if 0
					/* Don't bother with this during ldso initilization... */
					newvalue = fix_bad_pc24(reloc_addr, symbol_addr)
						- (unsigned long)reloc_addr + (addend << 2);
					topbits = newvalue & 0xfe000000;
					if (unlikely(topbits != 0xfe000000 && topbits != 0x00000000))
					{
						SEND_STDERR("R_ARM_PC24 relocation out of range\n");
						_dl_exit(1);
					}
#else
					SEND_STDERR("R_ARM_PC24 relocation out of range\n");
					_dl_exit(1);
#endif
				}
				newvalue >>= 2;
				symbol_addr = (*reloc_addr & 0xff000000) | (newvalue & 0x00ffffff);
				*reloc_addr = symbol_addr;
				break;
			}
		case R_ARM_GLOB_DAT:
		case R_ARM_JUMP_SLOT:
			/*
			 * Workaround for weak undefined references which need
			 * to stay NULL
			 */
			if (symtab && symtab->st_value == 0)
				*reloc_addr = 0;
			else
				*reloc_addr = symbol_addr;
			break;
		case R_ARM_RELATIVE:
			*reloc_addr += load_addr;
			break;
		case R_ARM_COPY:
			break;
#ifdef IN_RELOC_STATIC_PIE
		case R_ARM_TLS_DTPOFF32:
		case R_ARM_TLS_TPOFF32:
			/* Offsets are not affected by load address */
			break;
#endif
		default:
			SEND_STDERR("Unsupported relocation type\n");
			_dl_exit(1);
	}
}
