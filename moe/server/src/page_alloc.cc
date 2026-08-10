/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/util/util.h>

#include <l4/cxx/buddy_alloc>
#include <l4/cxx/iostream>
#include <l4/cxx/exceptions>
#include "page_alloc.h"
#include "debug.h"

#if 1
enum { page_alloc_debug = 0 };
#else
unsigned page_alloc_debug = 0;
#endif

using Page_alloc = cxx::Buddy_alloc;

static Page_alloc *page_alloc()
{
  static Page_alloc pa;
  return &pa;
}

bool Single_page_alloc_base::can_free = false;
Single_page_alloc_base::Config Single_page_alloc_base::default_mem_cfg;

Single_page_alloc_base::Single_page_alloc_base()
{}

size_t
Single_page_alloc_base::_avail()
{
  return page_alloc()->avail();
}

void *
Single_page_alloc_base::_alloc_max(size_t min, size_t *max, size_t align,
                                   size_t granularity, Config cfg)
{
  void *ret = nullptr;
  // TODO: If there are multiple regions an alloc_max might allocate less memory
  //       than actually would have been possible in a following region.
  for (auto const &region : cfg.regions)
    {
      ret = page_alloc()->alloc_max(min, max, align, granularity,
                                    region.range.start, region.range.end);
      if (ret)
        break; // successful allocation
    }

  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): alloc_max(" << *max << ") @" << ret << '\n';
  return ret;
}

void *
Single_page_alloc_base::_alloc(Nothrow, size_t size, size_t align, Config cfg)
{
  void *ret = nullptr;
  for (auto const &region : cfg.regions)
    {
      ret = page_alloc()->alloc(size, align, region.range.start, region.range.end);
      if (ret)
        break; // successful allocation
    }

  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): alloc(" << size << ") @" << ret << '\n';
  return ret;
}

void
Single_page_alloc_base::_free(void *p, size_t size)
{
  if (!can_free)
    return;

  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): free(" << size << ") @" << p << '\n';
  page_alloc()->free(p, size);
}

void
Single_page_alloc_base::_add_mem(void *p, size_t size)
{
  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): add_mem(" << size << ") @" << p << '\n';
  page_alloc()->add_mem(p, size);
}

size_t
Single_page_alloc_base::_metadata_bytes(l4_addr_t min_addr, l4_addr_t max_addr)
{
  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): metadata_bytes("
             << min_addr << ", " << max_addr << ")" << '\n';
  return Page_alloc::metadata_bytes(min_addr, max_addr);
}

void
Single_page_alloc_base::_init(l4_addr_t min_addr, l4_addr_t max_addr,
                              unsigned char *metadata_addr, size_t metadata_size)
{
  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): init(" << min_addr
             << ", " << max_addr << ", " << metadata_addr << ", " << metadata_size
             << ")" << '\n';
  page_alloc()->init(min_addr, max_addr, metadata_addr, metadata_size);
}

#ifndef NDEBUG
void
Single_page_alloc_base::_dump_free(Dbg &dbg)
{
  page_alloc()->dump_free(dbg);
}
#endif
