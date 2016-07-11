/* vi: set sw=4 ts=4: */
/* ARM ELF shared library loader suppport
 *
 * Copyright (C) 2001-2004 Erik Andersen
 * Copyright (C) 2016 Alexander Warg
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the above contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Program to load an ELF binary on a linux system, and run it.
   References to symbols in sharable libraries can be resolved by either
   an ELF sharable library or a linux style of shared library. */

/* Disclaimer:  I have never seen any AT&T source code for SVr4, nor have
   I ever taken any courses on internals.  This program was developed using
   information available through the book "UNIX SYSTEM V RELEASE 4,
   Programmers guide: Ansi C and Programming Support Tools", which did
   a more than adequate job of explaining everything required to get this
   working. */

#include "ldso.h"
#include "dl-tlsdesc-htab.h"

extern int _dl_linux_resolve(void);

unsigned long _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
	ELF_RELOC *this_reloc = (ElfW(Rela) *)(tpnt->dynamic_info[DT_JMPREL] + reloc_entry);

	int symtab_index = ELF_R_SYM(this_reloc->r_info);

	ElfW(Sym) const *symtab = (ElfW(Sym) const *)tpnt->dynamic_info[DT_SYMTAB];
	char const *strtab = (char const *)tpnt->dynamic_info[DT_STRTAB];
	char const *symname = strtab + symtab[symtab_index].st_name;

	/* Address of jump instruction to fix up */
	unsigned long instr_addr = ((unsigned long)this_reloc->r_offset
	                            + (unsigned long)tpnt->loadaddr);
	char **got_addr = (char **)instr_addr;

	/* Get the address of the GOT entry */
	unsigned long new_addr
		= (unsigned long)_dl_find_hash(symname, &_dl_loaded_modules->symbol_scope,
	                                   tpnt, ELF_RTYPE_CLASS_PLT, NULL);
	if (unlikely(!new_addr)) {
		_dl_dprintf(2, "%s: can't resolve symbol '%s'\n",
		            _dl_progname, symname);
		_dl_exit(1);
	}
#if defined (__SUPPORT_LD_DEBUG__)
# if !defined __SUPPORT_LD_DEBUG_EARLY__
	if ((unsigned long)got_addr < 0x40000000)
# endif
	{
		if (_dl_debug_bindings)
		{
			_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
			if (_dl_debug_detail)
				_dl_dprintf(_dl_debug_file, "\tpatch %p ==> %zx @ %p",
				            *got_addr, new_addr, got_addr);
		}
	}
	if (!_dl_debug_nofixups) {
		*got_addr = (char *)new_addr;
	}
#else
	*got_addr = (char *)new_addr;
#endif

	return new_addr;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct r_scope_elem *scope,
          unsigned long rel_addr, unsigned long rel_size,
          int (*reloc_fnc)(struct elf_resolve *tpnt, struct r_scope_elem *scope,
                           ElfW(Rela) *rpnt, ElfW(Sym) const *symtab,
                           char const *strtab))
{
	int goof = 0;

	/* Now parse the relocation information */
	ElfW(Rela) *rpnt = (ElfW(Rela) *)rel_addr;
	rel_size = rel_size / sizeof (ElfW(Rela));

	ElfW(Sym) *symtab = (ElfW(Sym) *)tpnt->dynamic_info[DT_SYMTAB];
	char const *strtab = (char const *)tpnt->dynamic_info[DT_STRTAB];

	unsigned long i;
	for (i = 0; i < rel_size; i++, rpnt++)
	{
		int res;

		int symtab_index = ELF_R_SYM(rpnt->r_info);

		debug_sym(symtab, strtab, symtab_index);
		debug_reloc(symtab, strtab, rpnt);

		res = reloc_fnc(tpnt, scope, rpnt, symtab, strtab);

		if (res == 0)
			continue;

		_dl_dprintf(2, "\n%s: ", _dl_progname);

		if (symtab_index)
			_dl_dprintf(2, "symbol '%s': ",
			            strtab + symtab[symtab_index].st_name);

		if (unlikely(res <0))
		{
		        int reloc_type = ELF_R_TYPE(rpnt->r_info);
#if defined (__SUPPORT_LD_DEBUG__)
			_dl_dprintf(2, "can't handle reloc type %s\n ", _dl_reltypes(reloc_type));
#else
			_dl_dprintf(2, "can't handle reloc type %x\n", reloc_type);
#endif
			_dl_exit(-res);
		}

		if (unlikely(res > 0))
		{
			_dl_dprintf(2, "can't resolve symbol\n");
			goof += res;
		}
	}
	return goof;
}

static int
_dl_do_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
             ElfW(Rela) *rpnt, ElfW(Sym) const *symtab, char const *strtab)
{
	struct elf_resolve *def_mod = 0;
	int goof = 0;

	unsigned long *reloc_addr
		= (unsigned long *)(tpnt->loadaddr + (unsigned long)rpnt->r_offset);


	int reloc_type = ELF_R_TYPE(rpnt->r_info);
	int symtab_index = ELF_R_SYM(rpnt->r_info);
	unsigned long symbol_addr = 0;
	struct symbol_ref sym_ref;
	sym_ref.sym = &symtab[symtab_index];
	sym_ref.tpnt = NULL;
	char const *symname = strtab + symtab[symtab_index].st_name;

	if (symtab_index)
	{
		symbol_addr = (unsigned long)_dl_find_hash(symname, scope, tpnt,
		                                           elf_machine_type_class(reloc_type),
		                                           &sym_ref);

		/*
		 * We want to allow undefined references to weak symbols - this might
		 * have been intentional.  We should not be linking local symbols
		 * here, so all bases should be covered.
		 */
		if (!symbol_addr && (ELF_ST_TYPE(symtab[symtab_index].st_info) != STT_TLS)
		    && (ELF_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK))
		{
			/* This may be non-fatal if called from dlopen.  */
			return 1;
		}

		if (_dl_trace_prelink)
		{
			_dl_debug_lookup(symname, tpnt, &symtab[symtab_index],
			                 &sym_ref, elf_machine_type_class(reloc_type));
		}

		def_mod = sym_ref.tpnt;
	} else {
		/*
		 * Relocs against STN_UNDEF are usually treated as using a
		 * symbol value of zero, and using the module containing the
		 * reloc itself.
		 */
		symbol_addr = symtab[symtab_index].st_value;
		def_mod = tpnt;
	}

#if defined (__SUPPORT_LD_DEBUG__)
	{
		unsigned long old_val = *reloc_addr;
#endif
		switch (reloc_type) {
			case R_AARCH64_NONE:
				break;
			case R_AARCH64_GLOB_DAT:
			case R_AARCH64_JUMP_SLOT:
			case R_AARCH64_ABS16:
			case R_AARCH64_ABS32:
			case R_AARCH64_ABS64:
				*reloc_addr = symbol_addr + rpnt->r_addend;
				break;
			case R_AARCH64_RELATIVE:
				*reloc_addr = (unsigned long) tpnt->loadaddr + rpnt->r_addend;
				break;
			case R_AARCH64_COPY:
				if (symbol_addr == 0)
					break;
				_dl_memcpy((void *)reloc_addr,
				           (void *)symbol_addr, symtab[symtab_index].st_size);
				break;
#if defined USE_TLS && USE_TLS
			case R_AARCH64_TLSDESC:
			{
				struct tlsdesc volatile *d = (struct tlsdesc volatile *)reloc_addr;
				d->arg = (void *)rpnt;
				d->entry = (void *)(tpnt->dynamic_info[DT_TLSDESC_PLT_IDX] + tpnt->loadaddr);
				break;
			}

			case R_AARCH64_TLS_DTPMOD:
				*reloc_addr = def_mod->l_tls_modid;
				break;

			case R_AARCH64_TLS_DTPREL:
				*reloc_addr = symbol_addr + rpnt->r_addend;
				break;

			case R_AARCH64_TLS_TPREL:
				CHECK_STATIC_TLS ((struct link_map *) def_mod);
				*reloc_addr = (symbol_addr + rpnt->r_addend + def_mod->l_tls_offset);
				break;
#endif
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_reloc && _dl_debug_detail)
			_dl_dprintf(_dl_debug_file, "\tpatch: %zx ==> %zx @ %p",
			            old_val, *reloc_addr, reloc_addr);
	}

#endif

	return goof;
}

static int
_dl_do_lazy_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
                  ELF_RELOC *rpnt, ElfW(Sym) const *symtab, char const *strtab)
{
	(void) scope;
	(void) symtab;
	(void) strtab;

	unsigned long *reloc_addr
		= (unsigned long *)(tpnt->loadaddr + (unsigned long)rpnt->r_offset);
	int reloc_type = ELF_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	{
		unsigned long old_val = *reloc_addr;
#endif
		switch (reloc_type) {
			case R_AARCH64_NONE:
				break;
			case R_AARCH64_JUMP_SLOT:
				*reloc_addr += (unsigned long)tpnt->loadaddr;
				break;
			case R_AARCH64_TLSDESC:
			{
				struct tlsdesc volatile *d = (struct tlsdesc volatile *)reloc_addr;
				d->arg = (void *)rpnt;
				d->entry = (void *)(tpnt->dynamic_info[DT_TLSDESC_PLT_IDX] + tpnt->loadaddr);
				break;
			}
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_reloc && _dl_debug_detail)
			_dl_dprintf(_dl_debug_file, "\tpatch: %zx ==> %zx @ %p",
			            old_val, *reloc_addr, reloc_addr);
	}

#endif
	return 0;

}

void
_dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
                                      unsigned long rel_addr,
                                      unsigned long rel_size)
{
	(void)_dl_parse(rpnt->dyn, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
}

int
_dl_parse_relocation_information(struct dyn_elf *rpnt,
                                 struct r_scope_elem *scope,
                                 unsigned long rel_addr,
                                 unsigned long rel_size)
{
	return _dl_parse(rpnt->dyn, scope, rel_addr, rel_size, _dl_do_reloc);
}

static __always_inline void
elf_machine_setup(ElfW(Addr) load_off, unsigned long const *dynamic_info,
                  struct elf_resolve *tpnt, int lazy)
{
	unsigned long *lpnt = (unsigned long *) dynamic_info[DT_PLTGOT];
#ifdef ALLOW_ZERO_PLTGOT
	if (lpnt)
#endif
		INIT_GOT(lpnt, tpnt);

	if (dynamic_info[DT_TLSDESC_GOT_IDX] && lazy)
		*(ElfW(Addr)*)(dynamic_info[DT_TLSDESC_GOT_IDX] + load_off)
			= (ElfW(Addr)) &_dl_tlsdesc_resolve_rela;
}

void
attribute_hidden
_dl_tlsdesc_resolver_rela(struct tlsdesc *td, struct elf_resolve *tpnt);

void
attribute_hidden
_dl_tlsdesc_resolver_rela(struct tlsdesc *td, struct elf_resolve *tpnt)
{
	const ElfW(Rela) *r = __atomic_load_n(&td->arg, __ATOMIC_RELAXED);

	if (_dl_tlsdesc_resolver_early_return(td, (void *)(tpnt->dynamic_info[DT_TLSDESC_PLT_IDX] + tpnt->loadaddr)))
		return;

	int symtab_index = ELF_R_SYM(r->r_info);

	ElfW(Sym) const *symtab = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB];
	char const *strtab = (char const *) tpnt->dynamic_info[DT_STRTAB];
	struct symbol_ref sym_ref;
	sym_ref.sym = &symtab[symtab_index];
	sym_ref.tpnt = NULL;
	char const *symname = strtab + sym_ref.sym->st_name;

	if (ELFW(ST_BIND)(sym_ref.sym->st_info) != STB_LOCAL
	    && ELFW(ST_VISIBILITY)(sym_ref.sym->st_other) == 0)
	{
		unsigned long new_addr
			= (unsigned long)_dl_find_hash(symname, &_dl_loaded_modules->symbol_scope,
			                               tpnt, ELF_RTYPE_CLASS_PLT, &sym_ref);
		if (unlikely(!new_addr)) {
			_dl_dprintf(2, "%s: can't resolve symbol '%s'\n",
				_dl_progname, symname);
			_dl_exit(1);
		}
	} else
	{
		sym_ref.tpnt = tpnt;
	}

	if (!sym_ref.sym)
	{
		__atomic_store_n(&td->arg, (void *)r->r_addend, __ATOMIC_RELAXED);
		__atomic_store_n(&td->entry, _dl_tlsdesc_undefweak, __ATOMIC_RELEASE);
	} else
	{
#ifndef SHARED
		CHECK_STATIC_TLS((struct link_map *)sym_ref.tpnt);
#else
		if (!TRY_STATIC_TLS((struct link_map *)sym_ref.tpnt))
		{
			void *p = _dl_make_tlsdesc_dynamic(sym_ref.tpnt, sym_ref.sym->st_value + r->r_addend);
			__atomic_store_n(&td->arg, p, __ATOMIC_RELAXED);
			__atomic_store_n(&td->entry, _dl_tlsdesc_dynamic, __ATOMIC_RELEASE);
		} else
#endif
		{
			void *p = (void*) (sym_ref.sym->st_value + sym_ref.tpnt->l_tls_offset
			                   + r->r_addend);
			__atomic_store_n(&td->arg, p, __ATOMIC_RELAXED);
			__atomic_store_n(&td->entry, _dl_tlsdesc_return_lazy, __ATOMIC_RELEASE);
		}
	}
	__rtld_lock_unlock_recursive (GL(dl_load_lock));
}

void
attribute_hidden
_dl_tlsdesc_resolver_block(struct tlsdesc *td, void *caller);

void
attribute_hidden
_dl_tlsdesc_resolver_block(struct tlsdesc *td, void *caller)
{
	/* Maybe we're lucky and can return early.  */
	if (caller != __atomic_load_n(&td->entry, __ATOMIC_RELAXED))
		return;

	__rtld_lock_lock_recursive (GL(dl_load_lock));
	__rtld_lock_unlock_recursive (GL(dl_load_lock));
}

