/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "dataspace_cont.h"

namespace Moe {

/**
 * Dynamically allocatable dataspace based on a contiguous region of RAM.
 *
 * This type of dataspace is always fully preallocated.
 */
class Dataspace_anon : public Dataspace_cont
{
public:
  Dataspace_anon(long size, Flags flags = L4Re::Dataspace::F::RWX,
                 unsigned char page_shift = L4_PAGESHIFT,
                 Single_page_alloc_base::Config cfg
                   = Single_page_alloc_base::Config());
  virtual ~Dataspace_anon();

  bool is_static() const noexcept override { return false; }
  int pre_allocate(l4_addr_t, l4_size_t, unsigned) override { return 0; }
};

};
