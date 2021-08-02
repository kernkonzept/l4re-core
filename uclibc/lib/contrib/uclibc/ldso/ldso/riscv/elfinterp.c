/* RISC-V ELF shared library loader suppport
 *
 * Copyright (C) 2011-2017 Free Software Foundation, Inc.
 * Copyright (C) 2021 Kernkonzept GmbH
 *
 * This file is part of the GNU C Library.
 *
 * The GNU C Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The GNU C Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the GNU C Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/* Partly adapted from glibc/sysdeps/riscv/dl-machine.h */

#include "ldso.h"

extern int _dl_runtime_resolve(void);

ElfW(Addr) _dl_runtime_resolver(struct elf_resolve *tpnt, int reloc_entry)
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
  ElfW(Addr) new_addr
    = (ElfW(Addr))_dl_find_hash(symname, &_dl_loaded_modules->symbol_scope,
                                     tpnt, ELF_RTYPE_CLASS_PLT, NULL);
  if (unlikely(!new_addr)) {
    _dl_dprintf(2, "%s: can't resolve symbol '%s'\n",
                _dl_progname, symname);
    _dl_exit(1);
  }
#if defined (__SUPPORT_LD_DEBUG__)
  if ((unsigned long)got_addr < 0x40000000)
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
      _dl_dprintf(2, "can't handle reloc type %x\n", reloc_type);
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
  ElfW(Addr) r_info = rpnt->r_info;
  const unsigned long int r_type = ELFW(R_TYPE) (r_info);
  ElfW(Addr) *addr_field
    = (ElfW(Addr) *) (tpnt->loadaddr + (unsigned long)rpnt->r_offset);

  int reloc_type = ELF_R_TYPE(rpnt->r_info);
  int symtab_index = ELF_R_SYM(rpnt->r_info);
  // Later: Resolved symbol in containing module
  const ElfW(Sym) *sym = &symtab[symtab_index];
  // Dynamic symbol in this module
  const ElfW(Sym) *const __attribute__((unused)) refsym = sym;
  // Module containing the resolved symbol
  struct elf_resolve *sym_map = 0;
  ElfW(Addr) value = 0;

  struct symbol_ref sym_ref;
  sym_ref.sym = sym;
  sym_ref.tpnt = NULL;
  char const *symname = strtab + sym->st_name;

  if (symtab_index)
  {
    // (TPNT)->loadaddr + (SYM)->st_value
    unsigned long symbol_addr = (unsigned long)_dl_find_hash(symname, scope, tpnt,
                                               elf_machine_type_class(reloc_type),
                                               &sym_ref);

    /*
     * We want to allow undefined references to weak symbols - this might
     * have been intentional.  We should not be linking local symbols
     * here, so all bases should be covered.
     */
    if (!symbol_addr && (ELF_ST_TYPE(sym->st_info) != STT_TLS)
        && (ELF_ST_BIND(sym->st_info) != STB_WEAK))
    {
      /* This may be non-fatal if called from dlopen.  */
      return 1;
    }

    if (_dl_trace_prelink)
    {
      _dl_debug_lookup(symname, tpnt, sym,
                       &sym_ref, elf_machine_type_class(reloc_type));
    }

    value = symbol_addr + rpnt->r_addend;
    sym = sym_ref.sym;
    sym_map = sym_ref.tpnt;
  } else {
    /*
     * Relocs against STN_UNDEF are usually treated as using a
     * symbol value of zero, and using the module containing the
     * reloc itself.
     */
    value = sym->st_value + rpnt->r_addend;
    sym_map = tpnt;
  }

#if defined (__SUPPORT_LD_DEBUG__)
  ElfW(Addr) old_val = *addr_field;
#endif

  switch (r_type)
  {
#if defined USE_TLS && USE_TLS
    case __WORDSIZE == 64 ? R_RISCV_TLS_DTPMOD64 : R_RISCV_TLS_DTPMOD32:
      if (sym_map)
        *addr_field = sym_map->l_tls_modid;
      break;

    case __WORDSIZE == 64 ? R_RISCV_TLS_DTPREL64 : R_RISCV_TLS_DTPREL32:
      if (sym != NULL)
        *addr_field = TLS_DTPREL_VALUE (sym) + rpnt->r_addend;
      break;

    case __WORDSIZE == 64 ? R_RISCV_TLS_TPREL64 : R_RISCV_TLS_TPREL32:
      if (sym != NULL)
        {
          CHECK_STATIC_TLS ((struct link_map *) sym_map);
          *addr_field = TLS_TPREL_VALUE (sym_map, sym) + rpnt->r_addend;
        }
      break;
#endif
    case R_RISCV_COPY:
    {
      if (__builtin_expect (sym == NULL, 0))
        /* This can happen in trace mode if an object could not be
           found.  */
        break;

      /* Handle TLS copy relocations.  */
      if (unlikely (ELFW(ST_TYPE) (sym->st_info) == STT_TLS))
      {
        /* There's nothing to do if the symbol is in .tbss.  */
        if (likely (sym->st_value >= sym_map->l_tls_initimage_size))
          break;
        value += (ElfW(Addr)) sym_map->l_tls_initimage - sym_map->loadaddr;
      }

      size_t size = sym->st_size;
      if (__builtin_expect (sym->st_size != refsym->st_size, 0))
      {
        if (sym->st_size > refsym->st_size)
          size = refsym->st_size;
#if defined (__SUPPORT_LD_DEBUG__)
        if (sym->st_size > refsym->st_size && _dl_debug_reloc)
          _dl_dprintf (_dl_debug_file,
                       "Symbol `%s' has different size in shared object, \
                        consider re-linking\n", symname);
#endif
      }

      _dl_memcpy ((void *)addr_field, (void *)value, size);
      break;
    }

    case R_RISCV_RELATIVE:
      *addr_field = tpnt->loadaddr + rpnt->r_addend;
      break;

    case R_RISCV_JUMP_SLOT:
    case __WORDSIZE == 64 ? R_RISCV_64 : R_RISCV_32:
      *addr_field = value;
      break;

    case R_RISCV_NONE:
      break;

    default:
      return -1; /* bad reloc type */
  }

#if defined (__SUPPORT_LD_DEBUG__)
  if (_dl_debug_reloc && _dl_debug_detail)
    _dl_dprintf(_dl_debug_file, "\tpatch: %zx ==> %zx @ %p",
                old_val, *addr_field, addr_field);
#endif

  return 0;
}

static int
_dl_do_lazy_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
                  ELF_RELOC *rpnt, ElfW(Sym) const *symtab, char const *strtab)
{
  (void) scope;
  (void) symtab;
  (void) strtab;

  ElfW(Addr) *const reloc_addr = (void *) (tpnt->loadaddr + rpnt->r_offset);
  const unsigned int r_type = ELFW(R_TYPE) (rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
  ElfW(Addr) old_val = *reloc_addr;
#endif

  switch (r_type) {
    case R_RISCV_NONE:
      break;
    case R_RISCV_JUMP_SLOT:
      *reloc_addr += tpnt->loadaddr;
      break;
    default:
      return -1; /*call _dl_exit(1) */
  }

#if defined (__SUPPORT_LD_DEBUG__)
  if (_dl_debug_reloc && _dl_debug_detail)
    _dl_dprintf(_dl_debug_file, "\tpatch: %zx ==> %zx @ %p",
                old_val, *reloc_addr, reloc_addr);
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
  (void) load_off;
  (void) lazy;
  unsigned long *lpnt = (unsigned long *) dynamic_info[DT_PLTGOT];
#ifdef ALLOW_ZERO_PLTGOT
  if (lpnt)
#endif
    INIT_GOT(lpnt, tpnt);
}
