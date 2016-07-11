#pragma once

/* Type used to represent a TLS descriptor in the GOT.  */
struct tlsdesc
{
  ptrdiff_t (*entry) (struct tlsdesc *);
  void *arg;
};

/* Type used as the argument in a TLS descriptor for a symbol that
   needs dynamic TLS offsets.  */
struct tlsdesc_dynamic_arg
{
	tls_index tlsinfo;
	size_t gen_count;
};

extern ptrdiff_t attribute_hidden
_dl_tlsdesc_return_lazy(struct tlsdesc *);

extern ptrdiff_t attribute_hidden
_dl_tlsdesc_resolve_rela(struct tlsdesc *);

extern ptrdiff_t attribute_hidden
_dl_tlsdesc_resolve_block(struct tlsdesc *);

extern ptrdiff_t attribute_hidden
_dl_tlsdesc_undefweak (struct tlsdesc *);

# ifdef SHARED
extern ptrdiff_t attribute_hidden
_dl_tlsdesc_dynamic (struct tlsdesc *);

void *
internal_function
_dl_make_tlsdesc_dynamic (struct elf_resolve *map, size_t ti_offset);
#endif

