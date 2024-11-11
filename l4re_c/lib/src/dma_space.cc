/*
 * (c) 2014 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/c/dma_space.h>
#include <l4/re/dma_space>

L4_CV long
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


L4_CV long
l4re_dma_space_unmap(l4re_dma_space_t dma, l4re_dma_space_dma_addr_t dma_addr,
                     l4_size_t size, unsigned long attrs,
                     enum l4re_dma_space_direction dir) L4_NOTHROW
{
  L4::Cap<L4Re::Dma_space> d(dma);
  return d->unmap(dma_addr, size,
                  L4Re::Dma_space::Attributes::from_raw(attrs),
                  L4Re::Dma_space::Direction(dir));
}

L4_CV long
l4re_dma_space_associate(l4re_dma_space_t dma, l4_cap_idx_t task,
                         unsigned long attrs) L4_NOTHROW
{
  static_assert(L4RE_DMA_SPACE_COHERENT == 1 << L4Re::Dma_space::Coherent,
                "enum mismatch");
  static_assert(L4RE_DMA_SPACE_PHYS_SPACE == 1 << L4Re::Dma_space::Phys_space,
                "enum mismatch");
  L4::Cap<L4Re::Dma_space> d(dma);
  return d->associate(L4::Ipc::Cap<L4::Task>::from_ci(task),
                      L4Re::Dma_space::Space_attribs::from_raw(attrs));
}

L4_CV long
l4re_dma_space_disassociate(l4re_dma_space_t dma)
{
  L4::Cap<L4Re::Dma_space> d(dma);
  return d->disassociate();
}
