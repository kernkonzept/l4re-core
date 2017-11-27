/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "dataspace.h"

namespace Moe {

class Dataspace_noncont : public Dataspace
{
public:
  enum
  {
    Page_addr_mask = ~((1UL << 12)-1),
    Page_cow = 0x04UL,
  };

  class Page
  {
  private:
    unsigned long p;

  public:
    Page() throw() : p(0) {}
    void *operator * () const throw() { return (void*)(p & Page_addr_mask); }
    bool valid() const throw() { return p & Page_addr_mask;}
    unsigned long flags() const throw() { return p & ~Page_addr_mask; }

    void set(void *addr, unsigned long flags) throw()
    { p = ((unsigned long)addr & Page_addr_mask) | (flags & ~Page_addr_mask); }

    void flags(unsigned long add, unsigned long del = 0) throw()
    {
      p = (p & Page_addr_mask & ~(del & ~Page_addr_mask))
        | ((unsigned long)add & ~Page_addr_mask);
    }

    void addr(void *a) throw()
    { p = (p & ~Page_addr_mask) | ((unsigned long)a & Page_addr_mask); }
  };

  bool is_static() const throw() { return false; }

  Dataspace_noncont(unsigned long size, unsigned long flags = Writable) throw()
  : Dataspace(size, flags | Cow_enabled, L4_LOG2_PAGESIZE), _pages(0)
  {}

  virtual ~Dataspace_noncont() {}

  Address address(l4_addr_t offset,
                  Ds_rw rw = Writable, l4_addr_t hot_spot = 0,
                  l4_addr_t min = 0, l4_addr_t max = ~0) const;
  void unmap(bool ro = false) const throw();

  int pre_allocate(l4_addr_t offset, l4_size_t size, unsigned rights);

  virtual Page &page(unsigned long offs) const throw() = 0;
  virtual Page &alloc_page(unsigned long offs) const = 0;

  unsigned long num_pages() const throw()
  { return (size()+page_size()-1) / page_size(); }

#if 0
private:
  unsigned idx_of(unsigned long offset) const { return offset >> 12; }

  void *page(unsigned idx) const
  { return (void*)(pages[idx] & Page_addr_mask); }
  unsigned flags(unsigned idx) const { return pages[idx] & ~Page_addr_mask; }
  void page(unsigned idx, void *page, unsigned flags = 0) const
  {
    pages[idx] = (unsigned long)page & Page_addr_mask | flags
      & ~Page_addr_mask; 
  }
#endif

  void free_page(Page &p) const throw();
  void unmap_page(Page const &p, bool ro = false) const throw();

public:
  long clear(unsigned long offs, unsigned long size) const throw();

  static Dataspace_noncont *create(Q_alloc *q, unsigned long size,
                                   unsigned long flags = Writable);

protected:
  union
  {
    Page *_pages;
    Page _page;
  };

};
};
