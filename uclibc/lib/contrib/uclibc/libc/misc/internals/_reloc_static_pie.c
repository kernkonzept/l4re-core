
#if defined(__PIC__) && !defined(SHARED)

#define SEND_STDERR(x)
#define SEND_EARLY_STDERR_DEBUG(x)
#define SEND_STDERR_DEBUG(x)
#define _dl_exit(x) do { for (;;); } while (0)
#define NO_FUNCS_BEFORE_BOOTSTRAP
#define IN_RELOC_STATIC_PIE

#include <dl-elf.h>
#include <dl-hash.h>
#include "dl-bootstrap.h"

extern struct dl_phdr_info _dl_phdr_info;

void _reloc_static_pie(void) attribute_hidden;
void _reloc_static_pie(void)
{
  struct elf_resolve tpnt_tmp;
  struct elf_resolve *tpnt = &tpnt_tmp;
  ElfW(Addr) load_addr = elf_machine_load_address();
  if (!load_addr)
    return;

  ElfW(Addr) dyn_addr = load_addr + elf_machine_dynamic();
  ElfW(Dyn) *dpnt = (void*)dyn_addr;

  /*
   * Copied from dl-startup.c
   */
	_dl_memset(tpnt, 0, sizeof(struct elf_resolve));
	tpnt->loadaddr = load_addr;
	/* OK, that was easy.  Next scan the DYNAMIC section of the image.
	   We are only doing ourself right now - we will have to do the rest later */
	SEND_EARLY_STDERR_DEBUG("Scanning DYNAMIC section\n");
	tpnt->dynamic_addr = dpnt;
#if defined(NO_FUNCS_BEFORE_BOOTSTRAP)
	/* Some architectures cannot call functions here, must inline */
	__dl_parse_dynamic_info(dpnt, tpnt->dynamic_info, NULL, load_addr);
#else
	_dl_parse_dynamic_info(dpnt, tpnt->dynamic_info, NULL, load_addr);
#endif

	/*
	 * BIG ASSUMPTION: We assume that the dynamic loader does not
	 *                 have any TLS data itself. If this ever occurs
	 *                 more work than what is done below for the
	 *                 loader will have to happen.
	 */
#if defined(USE_TLS) && USE_TLS
	/* This was done by _dl_memset above. */
	/* tpnt->l_tls_modid = 0; */
# if NO_TLS_OFFSET != 0
	tpnt->l_tls_offset = NO_TLS_OFFSET;
# endif
#endif

	SEND_EARLY_STDERR_DEBUG("Done scanning DYNAMIC section\n");

#if defined(PERFORM_BOOTSTRAP_GOT)
	SEND_EARLY_STDERR_DEBUG("About to do specific GOT bootstrap\n");
	/* some arches (like MIPS) we have to tweak the GOT before relocations */
	PERFORM_BOOTSTRAP_GOT(tpnt);
#endif

#if !defined(PERFORM_BOOTSTRAP_GOT) || defined(__avr32__) || defined(__mips__)

	/* OK, now do the relocations.  We do not do a lazy binding here, so
	   that once we are done, we have considerably more flexibility. */
	SEND_EARLY_STDERR_DEBUG("About to do library loader relocations\n");

	{
		int indx;
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

			if (!indx && relative_count) {
				rel_size -= relative_count * sizeof(ELF_RELOC);
#ifdef __LDSO_PRELINK_SUPPORT__
				if (load_addr || !tpnt->dynamic_info[DT_GNU_PRELINKED_IDX])
#endif
					elf_machine_relative(load_addr, rel_addr, relative_count);
				rel_addr += relative_count * sizeof(ELF_RELOC);
			}

			/*
 			 * Since ldso is linked with -Bsymbolic, all relocs should be RELATIVE.  All archs
			 * that need bootstrap relocations need to define ARCH_NEEDS_BOOTSTRAP_RELOCS.
			 */
#if 1 //def ARCH_NEEDS_BOOTSTRAP_RELOCS
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
#if !defined(EARLY_STDERR_SPECIAL)
						char *strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
						(void) strtab; /* avoid warning if debug output is disabled */
						SEND_STDERR_DEBUG("relocating symbol: ");
						SEND_STDERR_DEBUG(strtab + sym->st_name);
						SEND_STDERR_DEBUG("\n");
#endif
					} else {
						SEND_STDERR_DEBUG("relocating unknown symbol\n");
					}
					/* Use this machine-specific macro to perform the actual relocation.  */
					PERFORM_BOOTSTRAP_RELOC(rpnt, reloc_addr, symbol_addr, load_addr, sym);
				}
			}
#else /* ARCH_NEEDS_BOOTSTRAP_RELOCS */
			if (rel_size) {
				SEND_EARLY_STDERR("Cannot continue, found non relative relocs during the bootstrap.\n");
				_dl_exit(14);
			}
#endif
		}
	}
#endif

  _dl_phdr_info.dlpi_addr = load_addr;
}
#endif
