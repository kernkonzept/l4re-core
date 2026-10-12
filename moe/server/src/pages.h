/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include "debug.h"
#include "mem_region_list.h"

#include <l4/bid_config.h>
#include <l4/util/atomic.h>
#include <l4/sys/consts.h>

#include <cassert>
#include <cstdlib>

namespace Moe {
namespace Pages {
#ifdef CONFIG_MMU
  /**
   * Reference counters for the pages of one contiguous range of memory.
   */
  struct Bank
  {
    Moe::Mem_range range;
    l4_uint32_t *ref_counts;  ///< One counter per page of this bank.

    l4_uint32_t &ref_count(l4_addr_t addr) const
    { return ref_counts[(addr - range.start) >> L4_PAGESHIFT]; }
  };

  /**
   * The memory banks, sorted by address.
   *
   * Memory that is not covered by a bank needs no reference counters, i.e.
   * holes in the physical memory layout cost nothing. The array is allocated
   * at startup with as many banks as the memory layout requires.
   */
  extern Bank *banks;
  extern unsigned num_banks;

  /// Last address covered by any bank (inclusive).
  inline
  l4_addr_t max_addr()
  {
    return banks[num_banks - 1].range.end;
  }

  inline
  l4_uint32_t &ref_count(void *addr)
  {
    l4_addr_t a = reinterpret_cast<l4_addr_t>(addr);

    for (unsigned i = 0; i < num_banks; ++i)
      if (banks[i].range.contains_addr(a))
        return banks[i].ref_count(a);

    // Every page Moe hands out is covered by a bank.
    Err(Err::Fatal).printf("no reference counter for page %lx, halt\n", a);
    exit(128);
  }

  inline
  unsigned long share(void *addr)
  { return l4util_inc32_res(&ref_count(addr)); }

  inline
  unsigned long unshare(void *addr)
  { return l4util_dec32_res(&ref_count(addr)); }
#else
  inline
  l4_uint32_t &ref_count(void * /*addr*/)
  {
    return *(l4_uint32_t*)nullptr;
  }

  inline
  unsigned long share(void * /* addr */)
  { return 0; }

  inline
  unsigned long unshare(void * /*addr*/)
  { return 1; }
#endif
};
};
