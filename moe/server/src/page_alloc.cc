/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/util/util.h>

#include <l4/cxx/iostream>
#include <l4/cxx/list_alloc>
#include <l4/cxx/exceptions>
#include <l4/sys/kdebug.h>
#include "page_alloc.h"
#include "debug.h"

#if 1
enum { page_alloc_debug = 0 };
#else
unsigned page_alloc_debug = 0;
#endif

class LA : public cxx::List_alloc
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

unsigned long Single_page_alloc_base::_avail()
{
  return page_alloc()->avail();
}

void *Single_page_alloc_base::_alloc_max(unsigned long min,
                                         unsigned long *max,
                                         unsigned align,
                                         unsigned granularity,
                                         Config cfg)
{
  void *ret = page_alloc()->alloc_max(min, max, align, granularity, cfg.physmin,
                                      cfg.physmax);
  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): alloc(" << *max << ") @" << ret << '\n';
  return ret;
}

void *Single_page_alloc_base::_alloc(Nothrow, unsigned long size,
                                     unsigned long align, Config cfg)
{
  void *ret = page_alloc()->alloc(size, align, cfg.physmin, cfg.physmax);
  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): alloc(" << size << ") @" << ret << '\n';
  return ret;
}

void Single_page_alloc_base::_free(void *p, unsigned long size, bool initial_mem)
{
  if (!initial_mem && !can_free)
    return;

  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): free(" << size << ") @" << p << '\n';
  page_alloc()->free(p, size, initial_mem);
}

#ifndef NDEBUG
void Single_page_alloc_base::_dump_free(Dbg &dbg)
{
  page_alloc()->dump_free_list(dbg);
}
#endif
