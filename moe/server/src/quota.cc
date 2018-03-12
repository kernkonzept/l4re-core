/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#include "quota.h"

#include <cstdio>
#include <cassert>

namespace Moe {

Moe_alloc *Moe_alloc::allocator()
{
  static Moe_alloc a;
  return &a;
}

void *Q_alloc::get_mem()
{
  if (_quota.alloc(L4_PAGESIZE))
    {
      void *p = Malloc_container::get_mem();
      if (!p)
        _quota.free(L4_PAGESIZE);
      return p;
    }

  return 0;
}

void Q_alloc::free_mem(void *page)
{
  Malloc_container::free_mem(page);
  _quota.free(L4_PAGESIZE);
}

void Q_alloc::reparent(Malloc_container *new_container)
{
  auto newq = dynamic_cast<Q_alloc *>(new_container);
  assert(newq != 0);
  newq->quota()->free(_quota.limit());

}


} // namespace
