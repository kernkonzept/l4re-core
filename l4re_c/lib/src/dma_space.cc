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
                L4Re::Dma_space::Attributes(attrs),
                L4Re::Dma_space::Direction(dir), dma_addr);
}


L4_CV l4_ret_t
l4re_dma_space_unmap(l4re_dma_space_t dma, l4re_dma_space_dma_addr_t dma_addr,
                     l4_size_t size, unsigned long attrs,
                     enum l4re_dma_space_direction dir) L4_NOTHROW
{
  L4::Cap<L4Re::Dma_space> d(dma);
  return d->unmap(dma_addr, size,
                  L4Re::Dma_space::Attributes(attrs),
                  L4Re::Dma_space::Direction(dir));
}


L4_CV l4_ret_t
l4re_dma_space_mgr_associate(l4re_dma_space_mgr_t dma_mgr,
                             l4re_dma_space_t dma_space,
                             l4_cap_idx_t dma_task,
                             enum l4re_dma_space_mgr_space_attribs attrs) L4_NOTHROW
{
  L4::Cap<L4Re::Dma_space_mgr> m(dma_mgr);
  L4::Cap<L4Re::Dma_space> d(dma_space);
  return m->associate(L4::Ipc::make_cap_rws(d),
                      L4::Ipc::Cap<L4::Task>::from_ci(dma_task),
                      L4Re::Dma_space_mgr::Space_attribs(attrs));
}

L4_CV l4_ret_t
l4re_dma_space_mgr_associate_phys(l4re_dma_space_mgr_t dma_mgr,
                                  l4re_dma_space_t dma_space,
                                  enum l4re_dma_space_mgr_space_attribs attrs) L4_NOTHROW
{
  L4::Cap<L4Re::Dma_space_mgr> m(dma_mgr);
  L4::Cap<L4Re::Dma_space> d(dma_space);
  return m->associate_phys(L4::Ipc::make_cap_rws(d),
                           L4Re::Dma_space_mgr::Space_attribs(attrs));
}

L4_CV l4_ret_t
l4re_dma_space_mgr_disassociate(l4re_dma_space_mgr_t dma_mgr,
                                l4re_dma_space_t dma_space) L4_NOTHROW
{
  L4::Cap<L4Re::Dma_space_mgr> m(dma_mgr);
  L4::Cap<L4Re::Dma_space> d(dma_space);
  return m->disassociate(L4::Ipc::make_cap_rws(d));
}

L4_CV l4_ret_t
l4re_dma_space_mgr_block_area(l4re_dma_space_mgr_t dma_mgr,
                              l4re_dma_space_t dma_space,
                              l4re_dma_space_dma_addr_t *addr,
                              l4re_dma_space_dma_size_t size,
                              l4re_dma_space_dma_addr_t max_addr,
                              enum l4re_dma_space_mgr_block_flags flags,
                              unsigned char align) L4_NOTHROW
{
  L4::Cap<L4Re::Dma_space_mgr> m(dma_mgr);
  L4::Cap<L4Re::Dma_space> d(dma_space);
  return m->block_area(L4::Ipc::make_cap_rws(d), addr, size, max_addr,
                       L4Re::Dma_space_mgr::Block_flags(flags), align);
}

L4_CV l4_ret_t
l4re_dma_space_mgr_set_limits(l4re_dma_space_mgr_t dma_mgr,
                              l4re_dma_space_t dma_space,
                              l4re_dma_space_dma_addr_t min_addr,
                              l4re_dma_space_dma_addr_t max_addr) L4_NOTHROW
{
  L4::Cap<L4Re::Dma_space_mgr> m(dma_mgr);
  L4::Cap<L4Re::Dma_space> d(dma_space);
  return m->set_limits(L4::Ipc::make_cap_rws(d), min_addr, max_addr);
}
