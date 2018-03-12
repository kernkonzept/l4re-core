/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef SIGMA0_PAGE_ALLOC_H__
#define SIGMA0_PAGE_ALLOC_H__

#include <l4/sys/consts.h>

#include <l4/cxx/list_alloc>
#include <l4/cxx/slab_alloc>

class Page_alloc_base
{
public:
  typedef cxx::List_alloc Alloc;
protected:
  static Alloc _alloc;
  static unsigned long _total;
public:
  static void init();
  static void free(void *m)
  {
    allocator()->free(m, L4_PAGESIZE);
    _total += L4_PAGESIZE;
  }

  static Alloc *allocator() { return &_alloc; }
  static unsigned long total() { return _total; }
};

template< typename T >
class Page_alloc : public Page_alloc_base
{
public:
  enum { can_free = 1 };
  T *alloc()
  { return (T*)_alloc.alloc(L4_PAGESIZE,L4_PAGESIZE); }

  void free(T* b)
  { _alloc.free(b, L4_PAGESIZE); }
};

template< typename Type >
class Slab_alloc 
: public cxx::Slab_static<Type, L4_PAGESIZE, 2, Page_alloc>
{};


#endif
