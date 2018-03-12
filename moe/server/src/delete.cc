/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/iostream>
#include <cstddef>

#include "quota.h"

void operator delete (void *p) throw()
{
  Moe::Malloc_container::from_ptr(p)->free(p);
}

void * operator new (size_t s)
{
  // XXX alignment?
  return Moe::Moe_alloc::allocator()->alloc(s, 8);
}
