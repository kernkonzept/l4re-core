/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/cxx/alloc.h>

namespace L4 {

  void Alloc_list::free(void *blk, unsigned long size)
  {
    Elem *n = reinterpret_cast<Elem*>(blk);
    Elem **c = &_free;
    while (*c)
      {
        if (reinterpret_cast<char*>(*c) + (*c)->size == blk)
        {
          (*c)->size += size;
          blk = 0;
          break;
        }

        if (reinterpret_cast<char*>(*c) > blk)
          break;

        c = &((*c)->next);
      }

    if (blk)
      {
        n->next = *c;
        n->size = size;
        *c = n;
      }

    while (*c
           && (*c)->next
           && reinterpret_cast<char *>(*c) + (*c)->size
              == reinterpret_cast<char *>((*c)->next))
      {
        (*c)->size += (*c)->next->size;
        (*c)->next = (*c)->next->next;
      }
  }

  void *Alloc_list::alloc(unsigned long size)
  {
    if (!_free)
      return 0;

    // best fit;
    Elem **min = 0;
    Elem **c = &_free;

    while(*c)
      {
        if ((*c)->size >= size && (!min || (*min)->size > (*c)->size))
          min = c;

        c = &((*c)->next);
      }

    if (!min)
      return 0;

    void *r;
    if ((*min)->size > size)
      {
        r = reinterpret_cast<char*>(*min) + ((*min)->size - size);
        (*min)->size -= size;
      }
    else
      {
        r = *min;
        *min = (*min)->next;
      }

    return r;
  }
}
