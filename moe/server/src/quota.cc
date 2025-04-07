/*
 * Copyright (C) 2015, 2024 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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


} // namespace
