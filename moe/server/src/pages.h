/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/util/atomic.h>
#include <l4/sys/consts.h>
#include <cassert>

namespace Moe {
namespace Pages {
  extern l4_addr_t base_addr;
  extern l4_addr_t max_addr;
  extern l4_uint32_t *pages;

  inline
  l4_uint32_t &ref_count(void *addr)
  {
    assert(l4_addr_t(addr) >= base_addr);
    assert(l4_addr_t(addr) <  max_addr);
    return pages[(l4_addr_t(addr) - base_addr) >> L4_PAGESHIFT];
  }

  inline 
  unsigned long share(void *addr)
  { return l4util_inc32_res(&ref_count(addr)); }

  inline
  unsigned long unshare(void *addr)
  { return l4util_dec32_res(&ref_count(addr)); }
};
};

