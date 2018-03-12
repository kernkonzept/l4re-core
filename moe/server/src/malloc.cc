/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#include <cstring>
#include <cstdio>
#include <cassert>

#include <l4/cxx/slist>
#include <l4/cxx/minmax>

#include "debug.h"
#include "malloc.h"
#include "page_alloc.h"

static Dbg info(Dbg::Info);

namespace Moe {

/**
 * A page with allocatable memory.
 *
 * Simple one-size-fits-all bin based implementation.
 */
class Malloc_page : public cxx::S_list_item
{
public:
  enum
  {
    Page_shift = L4_PAGESHIFT,
    Page_size = 1 << Page_shift,
    Inval_idx = 0xFF
  };

  Malloc_page(Moe::Malloc_container *container, size_t binshift)
  : _container(container), _bin_shift(binshift), _first_free(0), _used(0)
  {
    // include bin size and the one free bit in the computation
    _num_bins = (Page_size - sizeof(Malloc_page)) / (1 << binshift);
    auto *c = static_cast<unsigned char *>(ptr_of(0));

    // initialise freelist
    for (unsigned char i = 1; i < _num_bins; ++i, c -= (1 << binshift))
      *c = i;
    *c = Inval_idx;
  }

  Moe::Malloc_container *container() const
  { return _container; }

  void reparent(Moe::Malloc_container *c)
  { _container = c; }

  void *alloc(size_t shift) throw()
  {
    if (shift != _bin_shift || _used >= _num_bins)
      return 0;

    void *freeptr = ptr_of(_first_free);

    _first_free = *static_cast<unsigned char *>(freeptr);
    ++_used;

    return freeptr;
  }

  void free(void *block) throw()
  {
    assert(block >= start_data() && block < end_data());

    unsigned char freeidx = index_of(block);

    *static_cast<unsigned char *>(ptr_of(freeidx)) = _first_free;
    _first_free = freeidx;
    --_used;
  }

  bool unused() const
  { return _used == 0; }

  static Malloc_page *from_ptr(void const *p) throw()
  {
    l4_addr_t caddr = l4_trunc_size(l4_addr_t(p), Malloc_page::Page_shift);
    return reinterpret_cast<Malloc_page *>(caddr);
  }

private:
  void *ptr_of(long idx) const
  { return end_data() - ((idx + 1) << _bin_shift); }

  long index_of(void *ptr) const
  { return ((Page_size - l4_addr_t(ptr) + l4_addr_t(this)) >> _bin_shift) - 1; }

  char *start_data() const
  { return end_data() - _num_bins * (1 << _bin_shift); }

  char *end_data() const
  { return reinterpret_cast<char *>(l4_addr_t(this) + Page_size); }

  Moe::Malloc_container *_container;
  unsigned char _bin_shift;
  unsigned char _num_bins;
  unsigned char _first_free;
  unsigned char _used;
};

}

void *
Moe::Malloc_container::alloc(size_t size, size_t align) throw()
{
  if (0)
    printf("Malloc[%p]: alloc(%zu, %zu)\n", this, size, align);
  // make sure alignment will be ok
  size = cxx::max(size, align);
  // now find the next possible n^2 alignment
  size_t outsz = 4;
  while ((1UL << outsz) < size)
    {
      ++outsz;
      if (outsz > 10)
        return 0;
    }

  for (auto const pg : _pages)
    {
      void *p = pg->alloc(outsz);
      if (p)
        return p;
    }

  // nothing? try to allocate a new page
  void *np = get_mem();

  if (np)
    {
      if (0)
        printf("Malloc[%p]: create new backing page @ %p (sz=%zu)\n",
               this, np, outsz);
      Malloc_page *pg = new (np) Malloc_page(this, outsz);
      _pages.add(pg);
      void *p = pg->alloc(outsz);
      return p;
    }

  return 0;
}

void
Moe::Malloc_container::free(void *block) throw()
{
  if (0)
    printf("Malloc[%p]: free(%p)\n", this, block);

  auto *pg = Malloc_page::from_ptr(block);

  if (pg->container() != this)
    {
      info.printf("WARNING: free called on wrong allocator.\n");
      return;
    }

  pg->free(block);

  if (pg->unused())
    {
      for (auto it = _pages.begin(); it != _pages.end(); ++it)
        {
          if (*it == pg)
            {
              _pages.erase(it);
              free_mem(pg);
              return;
            }
        }
    }
}

void *
Moe::Malloc_container::get_mem()
{
  return Single_page_alloc_base::_alloc(Single_page_alloc_base::nothrow,
                                        Malloc_page::Page_size,
                                        Malloc_page::Page_size);
}

void
Moe::Malloc_container::free_mem(void *page)
{
  Single_page_alloc_base::_free(page, Malloc_page::Page_size);
}

void
Moe::Malloc_container::reparent(Malloc_container *new_container)
{
  while (!_pages.empty())
    {
      auto *front = _pages.pop_front();
      front->reparent(new_container);
      new_container->_pages.add(front);
    }
}

Moe::Malloc_container *
Moe::Malloc_container::from_ptr(void const *p) throw()
{
  return Malloc_page::from_ptr(p)->container();
}
