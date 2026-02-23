/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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
  l4_ret_t pre_allocate(L4Re::Dataspace::Offset, L4Re::Dataspace::Size,
                        unsigned) override
  { return 0; }
};

};
