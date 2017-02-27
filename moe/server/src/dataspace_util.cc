/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "dataspace_util.h"
#include "dataspace_noncont.h"
#include "pages.h"

#include <cstring>
#include <l4/cxx/iostream>
#include <l4/cxx/minmax>
#include <l4/sys/cache.h>

using cxx::min;
using Moe::Dataspace;
using Moe::Dataspace_noncont;

namespace {

unsigned long trunc_page(unsigned page_size, unsigned long addr)
{ return addr & ~(page_size-1); }

inline void 
__do_real_copy(Dataspace *dst, unsigned long &dst_offs,
    Dataspace const *src, unsigned long &src_offs, unsigned long sz)
{
  while (sz)
    {
      Dataspace::Address src_a = src->address(src_offs, Dataspace::Read_only);
      Dataspace::Address dst_a = dst->address(dst_offs, Dataspace::Writable);

      unsigned long b_sz = min(min(src_a.sz() - src_a.of(),
            dst_a.sz() - dst_a.of()), sz);

      memcpy(dst_a.adr(), src_a.adr(), b_sz);
      // FIXME: we should change the API to pass a flag for executable target pages,
      // and do the I cache coherence only in this case.
      // And we should change the cache API to allow for a single call to handle
      // I-cache coherency and D-cache writeback.
      l4_cache_coherent((l4_addr_t)dst_a.adr(), (l4_addr_t)dst_a.adr() + b_sz - 1);
      l4_cache_clean_data((l4_addr_t)dst_a.adr(), (l4_addr_t)dst_a.adr() + b_sz - 1);

      src_offs += b_sz;
      dst_offs += b_sz;
      sz -= b_sz;
    }
}

inline void 
__do_cow_copy(Dataspace_noncont *dst, unsigned long &dst_offs, unsigned dst_pg_sz,
    Dataspace const *src, unsigned long &src_offs, unsigned long sz)
{
  while (sz)
    {
      Dataspace::Address src_a = src->address(src_offs, Dataspace::Read_only);
      Dataspace_noncont::Page &dst_p = dst->alloc_page(dst_offs);
      dst->free_page(dst_p);
      void *src_p = (void*)trunc_page(dst_pg_sz,src_a.adr<unsigned long>());
      Moe::Pages::share(src_p);
      dst_p.set(src_p, Dataspace_noncont::Page_cow);

      src_offs += dst_pg_sz;
      dst_offs += dst_pg_sz;
      sz  -= dst_pg_sz;
    }
}

inline void 
__do_cow_copy2(Dataspace_noncont *dst, unsigned long &dst_offs, unsigned dst_pg_sz,
    Dataspace_noncont const *src, unsigned long &src_offs, unsigned long sz)
{
  //L4::cout << "real COW\n";
  while (sz)
    {
      Dataspace_noncont::Page &src_p = src->page(src_offs);
      Dataspace_noncont::Page *dst_p;
      if (src_p.valid())
        dst_p = &dst->alloc_page(dst_offs);
      else
        dst_p = &dst->page(dst_offs);

      dst->free_page(*dst_p);
      if (*src_p)
        {
          Moe::Pages::share(*src_p);
          if (!(src_p.flags() & Dataspace_noncont::Page_cow))
            {
              src->unmap_page(src_p, true);
              src_p.set(*src_p, src_p.flags() | Dataspace_noncont::Page_cow);
            }

          dst_p->set(*src_p, src_p.flags() | Dataspace_noncont::Page_cow);
        }

      src_offs += dst_pg_sz;
      dst_offs += dst_pg_sz;
      sz  -= dst_pg_sz;
    }
}

unsigned long 
__do_eager_copy(Dataspace *dst, unsigned long dst_offs,
    Dataspace const *src, unsigned long src_offs, unsigned long size)
{
  unsigned long dst_sz = dst->size();
  unsigned long src_sz = src->round_size();
  if (dst_offs >= dst_sz || src_offs >= src_sz)
    return 0;

  size = min(min(size, dst_sz - dst_offs), src_sz - src_offs);

  __do_real_copy(dst, dst_offs, src, src_offs, size);
  return size;
}


bool
__do_lazy_copy(Dataspace_noncont *dst, unsigned long dst_offs,
    Dataspace const *src, unsigned long src_offs, unsigned long &size)
{
  unsigned long dst_sz = dst->size();
  unsigned long src_sz = src->round_size();
  if (dst_offs >= dst_sz || src_offs >= src_sz)
    {
      size = 0;
      return true;
    }

  unsigned dst_pg_sz = dst->page_size();

  if (src->page_size() < dst_pg_sz)
    {
      //L4::cout << "page sizes do not map\n";
      return false;
    }

  unsigned long dst_align = dst_offs & (dst_pg_sz-1);

  if (dst_align != (src_offs & (dst_pg_sz-1)))
    {
        if (0)
          L4::cout << "alignment error " << L4::hex << src_offs 
                   << " " << dst_offs << L4::dec << '\n';

      return false;
    }
  if (0)
    L4::cout << "do copy on write\n";

  unsigned long copy_sz = size 
    = min(min(size, dst_sz - dst_offs), src_sz - src_offs);

  if (dst_align)
    {
      unsigned long cp_sz = min(copy_sz, dst_pg_sz - dst_align);
      copy_sz -= cp_sz;

      if (0)
        L4::cout << "ensure cow starts on page: cp=" << cp_sz << '\n';

      __do_real_copy(dst, dst_offs, src, src_offs, cp_sz);
    }

  unsigned long cow_sz = trunc_page(dst_pg_sz, copy_sz);
  unsigned long cp_sz = copy_sz - cow_sz;

  if (0)
    L4::cout << "cow_sz=" << cow_sz << "; cp_sz=" << cp_sz << '\n';

  __do_cow_copy(dst, dst_offs, dst_pg_sz, src, src_offs, cow_sz);
  __do_real_copy(dst, dst_offs, src, src_offs, cp_sz);

  return true;
}

bool
__do_lazy_copy2(Dataspace_noncont *dst, unsigned long dst_offs,
    Dataspace_noncont const *src, unsigned long src_offs, unsigned long &size)
{
  unsigned long dst_sz = dst->size();
  unsigned long src_sz = src->round_size();
  if (dst_offs >= dst_sz || src_offs >= src_sz)
    {
      size = 0;
      return true;
    }

  unsigned dst_pg_sz = dst->page_size();

  if (src->page_size() != dst_pg_sz)
    {
      //L4::cout << "page sizes do not map\n";
      return false;
    }

  unsigned long dst_align = dst_offs & (dst_pg_sz-1);

  if (dst_align != (src_offs & (dst_pg_sz-1)))
    {
      if (0)
        L4::cout << "alignment error " << L4::hex << src_offs
                 << " " << dst_offs << L4::dec << '\n';

      return false;
    }

  if (0)
    L4::cout << "do copy on write\n";

  unsigned long copy_sz = size 
    = min(min(size, dst_sz - dst_offs), src_sz - src_offs);

  if (dst_align)
    {
      unsigned long cp_sz = min(copy_sz, dst_pg_sz - dst_align);
      copy_sz -= cp_sz;

      if (0)
        L4::cout << "ensure cow starts on page: cp=" << cp_sz << '\n';

      __do_real_copy(dst, dst_offs, src, src_offs, cp_sz);
    }

  unsigned long cow_sz = trunc_page(dst_pg_sz, copy_sz);
  unsigned long cp_sz = copy_sz - cow_sz;

  if (0)
    L4::cout << "cow_sz=" << cow_sz << "; cp_sz=" << cp_sz << '\n';

  __do_cow_copy2(dst, dst_offs, dst_pg_sz, src, src_offs, cow_sz);
  __do_real_copy(dst, dst_offs, src, src_offs, cp_sz);

  return true;
}

}; // and local anon namespace

unsigned long 
Dataspace_util::copy(Dataspace *dst, unsigned long dst_offs,
    Dataspace const *src, unsigned long src_offs, unsigned long size)
{
  if (src->can_cow() && dst->can_cow())
    {
      if (!src->is_writable() && src->is_static())
        {
          Dataspace_noncont *nc = dynamic_cast<Dataspace_noncont*>(dst);
          if (nc && __do_lazy_copy(nc, dst_offs, src, src_offs, size))
            return size;
        }
      else
        {
          Dataspace_noncont *dst_n = dynamic_cast<Dataspace_noncont*>(dst);
          Dataspace_noncont const *src_n = dynamic_cast<Dataspace_noncont const *>(src);
          if (dst_n && src_n 
              && __do_lazy_copy2(dst_n, dst_offs, src_n, src_offs, size))
            return size;
        }
    }

  return __do_eager_copy(dst, dst_offs, src, src_offs, size);
}


