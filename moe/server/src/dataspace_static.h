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
 * Dataspace that exists over the lifetime of Moe.
 */
class Dataspace_static : public Dataspace_cont
{
public:
  Dataspace_static(void *start, unsigned long size,
                   Flags flags = L4Re::Dataspace::F::RW,
                   unsigned char page_shift = L4_PAGESHIFT)
  : Dataspace_cont(start, size, flags, page_shift)
  {
    add_ref(); // Prevent deletion forever
  }

  virtual ~Dataspace_static() noexcept {}
  l4_ret_t pre_allocate(L4Re::Dataspace::Offset, L4Re::Dataspace::Size,
                        unsigned) override
  { return 0; }
  bool is_static() const noexcept override { return true; }
};

};


