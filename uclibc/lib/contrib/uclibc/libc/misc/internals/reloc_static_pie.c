/* Support for relocating static PIE.
   Copyright (C) 2017-2022 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
#define IS_IN_rtld      // force inline function calls
#include <link.h>
#include <elf.h>
#include <dl-elf.h>

#include <ldso.h>
#if defined(__m68k__) || defined(__mips__) || defined(__xtensa__)
#include <dl-startup.h>
#endif

extern ElfW(Addr) _dl_load_base;

void
reloc_static_pie (ElfW(Addr) load_addr);

void
reloc_static_pie(ElfW(Addr) load_addr)
{
    int indx;
    ElfW(Addr) got;
    ElfW(Dyn) *dpnt;
    struct elf_resolve tpnt_tmp;
	struct elf_resolve *tpnt = &tpnt_tmp;

    DL_BOOT_COMPUTE_GOT(got);
    DL_BOOT_COMPUTE_DYN(dpnt, got, (DL_LOADADDR_TYPE)load_addr);

    _dl_memset(tpnt, 0, sizeof(struct elf_resolve));
    tpnt->loadaddr = load_addr;
    tpnt->dynamic_addr = dpnt;

    __dl_parse_dynamic_info(dpnt, tpnt->dynamic_info, NULL, load_addr);

#if defined(PERFORM_BOOTSTRAP_GOT)
	/* some arches (like MIPS) we have to tweak the GOT before relocations */
	PERFORM_BOOTSTRAP_GOT(tpnt);
#endif

#if !defined(__FDPIC__)
    DL_RELOCATE_RELR(tpnt);
#endif

#if defined(ELF_MACHINE_PLTREL_OVERLAP)
# define INDX_MAX 1
#else
# define INDX_MAX 2
#endif

    for (indx = 0; indx < INDX_MAX; indx++) {
        unsigned long rel_addr, rel_size;
        ElfW(Word) relative_count = tpnt->dynamic_info[DT_RELCONT_IDX];

        rel_addr = (indx ? tpnt->dynamic_info[DT_JMPREL] :
                           tpnt->dynamic_info[DT_RELOC_TABLE_ADDR]);
        rel_size = (indx ? tpnt->dynamic_info[DT_PLTRELSZ] :
			               tpnt->dynamic_info[DT_RELOC_TABLE_SIZE]);

        if (!rel_addr)
            continue;

        if((0 == indx) && relative_count) {
			rel_size -= relative_count * sizeof(ELF_RELOC);
            elf_machine_relative(load_addr, rel_addr, relative_count);
			rel_addr += relative_count * sizeof(ELF_RELOC);
        }

#ifdef ARCH_NEEDS_BOOTSTRAP_RELOCS
			{
				ELF_RELOC *rpnt;
				unsigned int i;
				ElfW(Sym) *sym;
				unsigned long symbol_addr;
				int symtab_index;
				unsigned long *reloc_addr;

				/* Now parse the relocation information */
				rpnt = (ELF_RELOC *) rel_addr;
				for (i = 0; i < rel_size; i += sizeof(ELF_RELOC), rpnt++) {
					reloc_addr = (unsigned long *) DL_RELOC_ADDR(load_addr, (unsigned long)rpnt->r_offset);
					symtab_index = ELF_R_SYM(rpnt->r_info);
					symbol_addr = 0;
					sym = NULL;
					if (symtab_index) {
						ElfW(Sym) *symtab;
						symtab = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB];
						sym = &symtab[symtab_index];
						symbol_addr = (unsigned long) DL_RELOC_ADDR(load_addr, sym->st_value);
					}
					/* Use this machine-specific macro to perform the actual relocation.  */
					PERFORM_BOOTSTRAP_RELOC(rpnt, reloc_addr, symbol_addr, load_addr, sym);
				}
			}
#endif
    }
    _dl_load_base = load_addr;
}
