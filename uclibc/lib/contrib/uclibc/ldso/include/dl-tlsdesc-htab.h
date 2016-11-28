#pragma once

#include "dl-tlsdesc.h"
#include "bits/libc-lock.h"

#ifdef SHARED

#include <inline-hashtab.h>

inline static int
hash_tlsdesc(void *p)
{
  struct tlsdesc_dynamic_arg *td = p;

  /* We know all entries are for the same module, so ti_offset is the
     only distinguishing entry.  */
  return td->tlsinfo.ti_offset;
}

inline static int
eq_tlsdesc(void *p, void *q)
{
  struct tlsdesc_dynamic_arg *tdp = p, *tdq = q;

  return tdp->tlsinfo.ti_offset == tdq->tlsinfo.ti_offset;
}

inline static size_t
map_generation(struct elf_resolve *map)
{
  size_t idx = map->l_tls_modid;
  struct dtv_slotinfo_list *listp = GL(dl_tls_dtv_slotinfo_list);

  /* Find the place in the dtv slotinfo list.  */
  do
    {
      /* Does it fit in the array of this list element?  */
      if (idx < listp->len)
        {
          /* We should never get here for a module in static TLS, so
             we can assume that, if the generation count is zero, we
             still haven't determined the generation count for this
             module.  */
          if ((struct elf_resolve *)(listp->slotinfo[idx].map) == map
              && listp->slotinfo[idx].gen)
            return listp->slotinfo[idx].gen;
          else
            break;
        }
      idx -= listp->len;
      listp = listp->next;
    }
  while (listp != NULL);

  /* If we get to this point, the module still hasn't been assigned an
     entry in the dtv slotinfo data structures, and it will when we're
     done with relocations.  At that point, the module will get a
     generation number that is one past the current generation, so
     return exactly that.  */
  return GL(dl_tls_generation) + 1;
}

void *
internal_function
_dl_make_tlsdesc_dynamic(struct elf_resolve *map, size_t ti_offset)
{
  __rtld_lock_lock_recursive(GL(dl_load_lock));

  struct hashtab *ht = map->l_arch.tlsdesc_htab;
  if (!ht)
    {
      ht = htab_create();
      if (!ht)
        {
          __rtld_lock_unlock_recursive(GL(dl_load_lock));
          return 0;
        }
      map->l_arch.tlsdesc_htab = ht;
    }

  struct tlsdesc_dynamic_arg *td, test;
  test.tlsinfo.ti_module = map->l_tls_modid;
  test.tlsinfo.ti_offset = ti_offset;
  void **entry = htab_find_slot(ht, &test, 1, hash_tlsdesc, eq_tlsdesc);
  if (!entry)
    {
      __rtld_lock_unlock_recursive(GL(dl_load_lock));
      return 0;
    }

  if (*entry)
    {
      td = *entry;
      __rtld_lock_unlock_recursive(GL(dl_load_lock));
      return td;
    }

  *entry = td = _dl_malloc(sizeof (struct tlsdesc_dynamic_arg));
  td->gen_count = map_generation(map);
  td->tlsinfo = test.tlsinfo;

  __rtld_lock_unlock_recursive(GL(dl_load_lock));
  return td;
}

#endif

static int
_dl_tlsdesc_resolver_early_return(struct tlsdesc volatile *td, void *caller)
{
  if (caller != __atomic_load_n(&td->entry, __ATOMIC_RELAXED))
    return 1;

  __rtld_lock_lock_recursive (GL(dl_load_lock));
  if (caller != __atomic_load_n(&td->entry, __ATOMIC_RELAXED))
    {
      __rtld_lock_unlock_recursive (GL(dl_load_lock));
      return 1;
    }

  __atomic_store_n(&td->entry, _dl_tlsdesc_resolve_block, __ATOMIC_RELAXED);

  return 0;
}

