/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/bid_config.h>
#include "dataspace.h"

namespace Moe {

/**
 * A dataspace serving a contiguous range of physical memory.
 */
class Dataspace_cont : public Dataspace
{
public:
  Dataspace_cont(void *start, unsigned long size,
                 Flags flags,
                 unsigned char page_shift);

  ~Dataspace_cont() { unmap(); }

  Address address(l4_addr_t offset,
                  Flags flags, l4_addr_t hot_spot = 0,
                  l4_addr_t min = 0, l4_addr_t max = ~0) const override;
  int copy_address(l4_addr_t offset, Flags flags,
                   l4_addr_t *addr, unsigned long *size) const override;

  int dma_map(Dma_space *dma, l4_addr_t offset, l4_size_t *size,
              Dma_attribs dma_attrs, Dma_space::Direction dir,
              Dma_space::Dma_addr *dma_addr) override;

#if !defined(CONFIG_MMU)
  long map_info(l4_addr_t &start_addr,
                l4_addr_t &end_addr) const noexcept override;
#endif

protected:
  void start(void *start) { _start = static_cast<char*>(start); }
  void *start() { return _start; }

private:
  void unmap() const noexcept;

  char *_start;
};

};

