/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include "dataspace_anon.h"
#include "page_alloc.h"
#include "debug.h"

#include <l4/cxx/exceptions>
#include <l4/cxx/minmax>

#include <l4/sys/cache.h>

#include <cstring>
#include <climits>

Moe::Dataspace_anon::Dataspace_anon(long size, Flags w,
                                    unsigned char page_shift,
                                    Single_page_alloc_base::Config cfg)
: Moe::Dataspace_cont(0, 0, w, page_shift, cfg)
{
  Quota_guard g;
  Single_page_unique_ptr m;

  // test whether the client requested to max out his allocation and whether
  // the allocator can fulfill this request in theory
  if (L4_UNLIKELY(size < 0))
    {
      unsigned long l = qalloc()->quota()->limit();
      unsigned long a = Single_page_alloc_base::_avail();

      a = cxx::min(l - qalloc()->quota()->reserved(), a);

      // not enough memory left
      if (a <= static_cast<unsigned long>(-size))
        L4Re::chksys(-L4_ENOMEM);

      if (l == ~0UL)
        l = LONG_MAX;

      if (l <= static_cast<unsigned long>(-size))
        L4Re::chksys(-L4_ENOMEM);

      size = cxx::min(a + size, l + size);
      size = l4_trunc_size(size, page_shift);

      if (size == 0L)
        L4Re::chksys(-L4_ENOMEM);

      unsigned long r_size = size;
      void *_m = Single_page_alloc_base::_alloc_max(page_size(), &r_size,
                                                    page_size(), page_size(),
                                                    cfg);

      if (!_m)
        L4Re::chksys(-L4_ENOMEM);

      m = Single_page_unique_ptr(_m, r_size);
      g = Quota_guard(qalloc()->quota(), r_size);
      size = r_size;
    }
  else
    {
      unsigned long r_size = (size + page_size() - 1) & ~(page_size() -1);
      g = Quota_guard(qalloc()->quota(), r_size);
      void *_m = Single_page_alloc_base::_alloc(r_size, page_size(), cfg);

      m = Single_page_unique_ptr(_m, r_size);
    }

  memset(m.get(), 0, m.size());
  // No need for I cache coherence, as we just zero fill and assume that
  // this is no executable code
  l4_cache_clean_data(reinterpret_cast<l4_addr_t>(m.get()),
                      reinterpret_cast<l4_addr_t>(m.get()) + m.size());

  start(m.release());
  this->size(size);
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
