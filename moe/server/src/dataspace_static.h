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

class Dataspace_static : public Dataspace_cont
{
public:
  Dataspace_static(void *start, unsigned long size, unsigned short flags = 0,
                   unsigned char page_shift = L4_PAGESHIFT)
  : Dataspace_cont(start, size, flags, page_shift) {}
  virtual ~Dataspace_static() throw() {}
  virtual int pre_allocate(l4_addr_t, l4_size_t, unsigned) { return 0; }
  bool is_static() const throw() { return true; }
};

};


