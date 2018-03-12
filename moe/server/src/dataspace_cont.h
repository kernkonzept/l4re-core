/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "dataspace.h"

namespace Moe {

class Dataspace_cont : public Dataspace
{
public:
  Dataspace_cont(void *start, unsigned long size, unsigned short flags,
                 unsigned char page_shift);

  Address address(l4_addr_t offset,
                  Ds_rw rw, l4_addr_t hot_spot = 0,
                  l4_addr_t min = 0, l4_addr_t max = ~0) const;

  void unmap(bool ro = false) const throw();

  int dma_map(Dma_space *dma, l4_addr_t offset, l4_size_t *size,
              Dma_attribs dma_attrs, Dma_space::Direction dir,
              Dma_space::Dma_addr *dma_addr);
  int dma_unmap(Dma_space *dma, l4_addr_t offset, l4_size_t size,
                Dma_attribs dma_attrs, Dma_space::Direction dir);

protected:
  void start(void *start) { _start = (char*)start; }
  void *start() { return _start; }

private:
  char *_start;
};

};

