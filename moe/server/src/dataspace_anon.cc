/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "dataspace_anon.h"
#include "page_alloc.h"
#include "debug.h"

#include <l4/cxx/exceptions>
#include <l4/cxx/minmax>

#include <cstring>
#include <climits>

Moe::Dataspace_anon::Dataspace_anon(long _size, bool w,
                                    unsigned char page_shift)
: Moe::Dataspace_cont(0, 0, w, page_shift)
{
  Quota_guard g;
  Single_page_unique_ptr m;

  // test whether the client requested to max out his allocation and whether
  // the allocator can fulfill this request in theory
  if (L4_UNLIKELY(_size < 0))
    {
      unsigned long l = qalloc()->quota()->limit();
      unsigned long a = Single_page_alloc_base::_avail();

      a = cxx::min(l - qalloc()->quota()->used(), a);

      // not enough memory left
      if (a <= (unsigned long)(-_size))
        L4Re::chksys(-L4_ENOMEM);

      if (l == ~0UL)
        l = LONG_MAX;

      if (l <= (unsigned long)(-_size))
        L4Re::chksys(-L4_ENOMEM);

      _size = cxx::min(a + _size, l + _size);
      _size = l4_trunc_size(_size, page_shift);

      if (_size == 0L)
        L4Re::chksys(-L4_ENOMEM);

      unsigned long r_size = _size;
      void *_m = Single_page_alloc_base::_alloc_max(page_size(), &r_size,
                                                    page_size(), page_size());

      if (!_m)
        L4Re::chksys(-L4_ENOMEM);

      m = Single_page_unique_ptr(_m, r_size);
      g = Quota_guard(qalloc()->quota(), r_size);
      _size = r_size;
    }
  else
    {
      unsigned long r_size = (_size + page_size() - 1) & ~(page_size() -1);
      g = Quota_guard(qalloc()->quota(), r_size);
      void *_m = Single_page_alloc_base::_alloc(r_size, page_size());

      m = Single_page_unique_ptr(_m, r_size);
    }

  memset(m.get(), 0, m.size());
  start(m.release());
  size(_size);
  g.release();
}

Moe::Dataspace_anon::~Dataspace_anon()
{
  void *adr = start();
  if (adr)
    {
      unsigned long r_size = (size() + page_size() - 1) & ~(page_size() -1);
      Single_page_alloc_base::_free(adr, r_size);
      qalloc()->quota()->free(r_size);
    }
}
