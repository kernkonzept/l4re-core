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

class Dataspace_anon : public Dataspace_cont
{
public:
  Dataspace_anon(long size, bool writable = true,
                 unsigned char page_shift = L4_PAGESHIFT);
  virtual ~Dataspace_anon();

  bool is_static() const throw() { return false; }
  int pre_allocate(l4_addr_t, l4_size_t, unsigned) { return 0; }
};

};
