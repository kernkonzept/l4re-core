/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#ifndef SIGMA0_MEM_MAN_H__
#define SIGMA0_MEM_MAN_H__

#include <l4/cxx/avl_set>

#include "page_alloc.h"
#include "region.h"


class Mem_man 
{
private:
  bool alloc_from(Region const *r2, Region const &r);

  static Mem_man _ram;

public:
  typedef cxx::Avl_set< Region, cxx::Lt_functor<Region>, Slab_alloc> Tree;

private:
  Tree _tree;

public:
  static Mem_man *ram() { return &_ram; }

  unsigned long alloc(Region const &r, bool force = false);
  bool reserve(Region const &r);
  bool add_free(Region const &r);
  bool add(Region const &r);
  Region const *find(Region const &r, bool force = false) const;

  bool morecore();
  unsigned long alloc_first(unsigned long size, unsigned owner = 2);

  void dump();
};

#endif

