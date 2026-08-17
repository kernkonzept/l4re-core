/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/util/util.h>

#include <l4/cxx/iostream>
#include <l4/cxx/exceptions>
#include "page_alloc.h"
#include "debug.h"

#if 1
enum { page_alloc_debug = 0 };
#else
unsigned page_alloc_debug = 0;
#endif

#ifdef CONFIG_MOE_PAGE_ALLOC_TREE
#include <l4/cxx/tree_alloc>
using Page_alloc = cxx::Tree_alloc;
#else
#include <l4/cxx/list_alloc>
using Page_alloc = cxx::List_alloc;
#endif

class LA : public Page_alloc
{
#if 0
public:
  ~LA()
    {
      L4::cout << "~LA(): avail = " << avail() << '\n';
    }
#endif
#if 0
public:
  void *alloc(unsigned long size, unsigned long align)
  {
    L4::cout << "PA::alloc: " << L4::hex << size << '(' << align << ") -> \n";
    void *p = cxx::List_alloc::alloc(size, align);
    L4::cout << p << "\n";
    return p;
  }
#endif
#if 0
public:
  void free(void *p, unsigned long size)
  {
    L4::cout << "free: " << p << '(' << size << ") -> ";
    cxx::List_alloc::free(p, size);
    L4::cout << avail() << "\n";
  }
#endif
};

static LA *page_alloc()
{
  static LA pa;
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
  unsigned long *max_ptr = reinterpret_cast<unsigned long *>(max);
  void *ret = page_alloc()->alloc_max(min, max_ptr, align, granularity,
                                      cfg.physmin, cfg.physmax);
  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): alloc_max(" << *max << ") @" << ret << '\n';
  return ret;
}

void *
Single_page_alloc_base::_alloc(Nothrow, size_t size, size_t align, Config cfg)
{
  void *ret = page_alloc()->alloc(size, align, cfg.physmin, cfg.physmax);
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
  page_alloc()->free(p, size, true);
}

#ifndef NDEBUG
void
Single_page_alloc_base::_dump_free(Dbg &dbg)
{
  page_alloc()->dump_free_list(dbg);
}
#endif
