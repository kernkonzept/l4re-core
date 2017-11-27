/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "dataspace_noncont.h"
#include "quota.h"
#include "pages.h"

#include <l4/sys/task.h>
#include <l4/sys/cache.h>

#include <l4/cxx/iostream>
#include <l4/cxx/minmax>
#include <l4/cxx/exceptions>
#include <cstring>
#include <climits>

using cxx::min;

void
Moe::Dataspace_noncont::unmap_page(Page const &p, bool ro) const throw()
{
  if (p.valid())
    l4_task_unmap(L4_BASE_TASK_CAP,
                  l4_fpage((unsigned long)*p, page_shift(),
                  ro ? L4_FPAGE_W : L4_FPAGE_RWX), L4_FP_OTHER_SPACES);
}

void
Moe::Dataspace_noncont::free_page(Page &p) const throw()
{
  unmap_page(p);
  if (p.valid() && !Moe::Pages::unshare(*p))
    {
      //L4::cout << "free page @" << *p << '\n';
      qalloc()->free_pages(*p, page_size());
    }

  p.set(0, 0);
}

Moe::Dataspace::Address
Moe::Dataspace_noncont::address(l4_addr_t offset,
                                Ds_rw rw, l4_addr_t,
                                l4_addr_t, l4_addr_t) const
{
  // XXX: There may be a problem with data spaces with
  //      page_size() > L4_PAGE_SIZE
  //      MUST review that!!
  if (!check_limit(offset))
    return Address(-L4_ERANGE);

  Page &p = alloc_page(offset);

  if (!is_writable())
    rw = Read_only;

  if ((rw == Writable) && (p.flags() & Page_cow))
    {
      if (Moe::Pages::ref_count(*p) == 1)
        p.set(*p, p.flags() & ~Page_cow);
      else
        {
          void *np = qalloc()->alloc_pages(page_size(), page_size());
          Moe::Pages::share(np);

          // L4::cout << "copy on write for " << *p << " to " << np << '\n';
          memcpy(np, *p, page_size());
          // FIXME: we should pass information if this page is to be mapped
          // executable or not and conditionally make I caches coherent.
          // And we should provide a single API with opcode bits to allow
          // a combination of cache clean and I cache coherency in a single
          // operation.
          l4_cache_coherent((l4_addr_t)np, (l4_addr_t)np + page_size() - 1);
          l4_cache_clean_data((l4_addr_t)np, (l4_addr_t)np + page_size() - 1);
          unmap_page(p);
          Moe::Pages::unshare(*p);
          p.set(np, 0);
        }
    }

  if (!*p)
    {
      p.set(qalloc()->alloc_pages(page_size(), page_size()), 0);
      Moe::Pages::share(*p);
      memset(*p, 0, page_size());
      // No need for I cache coherence, as we just zero fill and assume that
      // this is no executable code
      l4_cache_clean_data((l4_addr_t)*p, (l4_addr_t)(*p) + page_size() - 1);
    }

  return Address(l4_addr_t(*p), page_shift(), rw, offset & (page_size()-1));
}

int
Moe::Dataspace_noncont::pre_allocate(l4_addr_t offset, l4_size_t size, unsigned rights)
{
  if (!check_range(offset, size))
    return -L4_ERANGE;

  l4_addr_t end_off = l4_round_size(offset + size, page_shift());

  l4_size_t ps = page_size();
  for (l4_addr_t o = l4_trunc_size(offset, page_shift()); o < end_off; o += ps)
    {
      Address a = address(o, rights & L4_CAP_FPAGE_W ? Writable : Read_only);
      if (a.is_nil())
        return a.error();
    }
  return 0;
}

void
Moe::Dataspace_noncont::unmap(bool ro) const throw()
{
  for (unsigned long i = num_pages(); i > 0; --i)
    unmap_page(page((i - 1) << page_shift()), ro);
}

long
Moe::Dataspace_noncont::clear(unsigned long offs, unsigned long _size) const throw()
{
  if (!check_limit(offs))
    return -L4_ERANGE;

  unsigned long sz = _size = min(_size, round_size()-offs);
  unsigned long pg_sz = page_size();
  unsigned long pre_sz = offs & (pg_sz-1);
  if (pre_sz)
    {
      pre_sz = min(pg_sz - pre_sz, sz);
      Moe::Dataspace::clear(offs, pre_sz);
      sz -= pre_sz;
      offs += pre_sz;
    }

  unsigned long u_sz = sz & ~(pg_sz-1);

  while (u_sz)
    {
      // printf("ds free page offs %lx\n", offs);
      free_page(page(offs));
      offs += pg_sz;
      u_sz -= pg_sz;
    }

  sz &= (pg_sz-1);

  if (sz)
    Moe::Dataspace::clear(offs, sz);

  return 0;
}

namespace {
  class Mem_one_page : public Moe::Dataspace_noncont
  {
  public:
    Mem_one_page(unsigned long size, unsigned long flags) throw()
    : Moe::Dataspace_noncont(size, flags)
    {}

    ~Mem_one_page() throw()
    { free_page(page(0)); }

    Page &page(unsigned long /*offs*/) const throw()
    { return const_cast<Page &>(_page); }

    Page &alloc_page(unsigned long /*offs*/) const throw()
    { return const_cast<Page &>(_page); }
  };

  class Mem_small : public Moe::Dataspace_noncont
  {
    enum
    {
      Meta_align_bits = 10,
      Meta_align      = 1UL << Meta_align_bits,
    };

  public:
    unsigned long meta_size() const throw()
    { return (l4_round_size(num_pages()*sizeof(unsigned long), Meta_align_bits)); }
    Mem_small(unsigned long size, unsigned long flags)
    : Moe::Dataspace_noncont(size, flags)
    {
      _pages = (Page *)qalloc()->alloc_pages(meta_size(), Meta_align);
      memset(_pages, 0, meta_size());
    }

    ~Mem_small() throw()
    {
      for (unsigned long i = num_pages(); i > 0; --i)
        free_page(page((i - 1) << page_shift()));

      qalloc()->free_pages(_pages, meta_size());
    }

    Page &page(unsigned long offs) const throw()
    { return _pages[offs >> page_shift()]; }

    Page &alloc_page(unsigned long offs) const throw()
    { return _pages[offs >> page_shift()]; }

  };

  class Mem_big : public Moe::Dataspace_noncont
  {
  public:

    // use a 4KB second level for page management
    static unsigned long meta2_size() throw()
    { return 1UL << 12; }

    static unsigned long entries2() throw()
    { return meta2_size() / sizeof(Page *); }

  private:
    class L1
    {
    private:
      unsigned long p;

    public:
      Page *l2() const throw() { return (Page*)(p & ~0xfffUL); }
      Page &operator [] (unsigned idx) throw()
      { return l2()[idx]; }
      Page *operator * () const throw() { return l2(); }
      unsigned long cnt() const throw() { return p & 0xfffUL; }
      void inc() throw() { p = (p & ~0xfffUL) | (((p & 0xfffUL)+1) & 0xfffUL); }
      void dec() throw() { p = (p & ~0xfffUL) | (((p & 0xfffUL)-1) & 0xfffUL); }
      void set(void* _p) throw() { p = (unsigned long)_p; }
    };

    L1 &__p(unsigned long offs) const throw()
    { return ((L1*)_pages)[(offs >> page_shift()) / entries2()]; }

    unsigned l2_idx(unsigned long offs) const
    { return (offs >> page_shift()) & (entries2() - 1); }

  public:
    unsigned long entries1() const throw()
    { return (num_pages() + entries2() - 1) / entries2(); }

    long meta1_size() const throw()
    { return l4_round_size(entries1() * sizeof(L1 *), 10); }

    Mem_big(unsigned long size, unsigned long flags)
    : Moe::Dataspace_noncont(size, flags)
    {
      _pages = (Page *)qalloc()->alloc_pages(meta1_size(), 1024);
      memset(_pages, 0, meta1_size());
    }

    ~Mem_big() throw()
    {
      for (unsigned long i = 0; i < size(); i += page_size())
        free_page(page(i));

      for (L1 *p = (L1 *)_pages; p != (L1 *)_pages + entries1(); ++p)
        {
          if (**p)
            qalloc()->free_pages(**p, meta2_size());
          p->set(0);
        }

      qalloc()->free_pages(_pages, meta1_size());
    }

    Page &page(unsigned long offs) const throw()
    {
      static Page invalid_page;
      if (!*__p(offs))
        return invalid_page;

      return __p(offs)[l2_idx(offs)];
    }

    Page &alloc_page(unsigned long offs) const
    {
      L1 &_p = __p(offs);
      if (!*_p)
        {
          void *a = qalloc()->alloc_pages(meta2_size(), meta2_size());
          assert (((l4_addr_t)a & 0xfff) == 0);
          _p.set(a);
          memset(a, 0, meta2_size());
        }

      return _p[l2_idx(offs)];
    }
  };
};


Moe::Dataspace_noncont *
Moe::Dataspace_noncont::create(Moe::Q_alloc *q, unsigned long size,
                               unsigned long flags)
{
  if (size <= L4_PAGESIZE)
    return q->make_obj<Mem_one_page>(size, flags);
  else if (size <= L4_PAGESIZE * (L4_PAGESIZE / sizeof(unsigned long)))
    return q->make_obj<Mem_small>(size, flags);
  else
    return q->make_obj<Mem_big>(size, flags);
}

