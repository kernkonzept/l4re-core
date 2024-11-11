/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <cstddef>

#include "quota.h"

void operator delete (void *p) noexcept
{
  Moe::Malloc_container::from_ptr(p)->free(p);
}

#if __cplusplus >= 201400
void operator delete (void *p, size_t) noexcept
{
  Moe::Malloc_container::from_ptr(p)->free(p);
}
#endif

void * operator new (size_t s)
{
  // XXX alignment?
  return Moe::Moe_alloc::allocator()->alloc(s, 8);
}
