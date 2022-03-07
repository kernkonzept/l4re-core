/*
 * (c) 2014 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/c/dma_space.h>
#include <l4/re/dma_space>

L4_CV l4_ret_t
l4re_dma_space_map(l4re_dma_space_t dma, l4re_ds_t src,
                   l4re_ds_offset_t offset,
                   l4_size_t * size, unsigned long attrs,
                   enum l4re_dma_space_direction dir,
                   l4re_dma_space_dma_addr_t *dma_addr) L4_NOTHROW
{
  L4::Cap<L4Re::Dma_space> d(dma);
  return d->map(L4::Ipc::Cap<L4Re::Dataspace>::from_ci(src),
                offset, size,
                L4Re::Dma_space::Attributes::from_raw(attrs),
                L4Re::Dma_space::Direction(dir), dma_addr);
}


L4_CV l4_ret_t
l4re_dma_space_unmap(l4re_dma_space_t dma, l4re_dma_space_dma_addr_t dma_addr,
                     l4_size_t size, unsigned long attrs,
                     enum l4re_dma_space_direction dir) L4_NOTHROW
{
  L4::Cap<L4Re::Dma_space> d(dma);
  return d->unmap(dma_addr, size,
                  L4Re::Dma_space::Attributes::from_raw(attrs),
                  L4Re::Dma_space::Direction(dir));
}

