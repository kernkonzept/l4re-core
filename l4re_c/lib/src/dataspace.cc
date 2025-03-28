/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/re/c/dataspace.h>

#include <l4/re/mem_alloc>
#include <l4/re/dataspace>
#include <l4/re/env>
#include <l4/sys/err.h>


int
l4re_ds_map(l4re_ds_t ds,
            l4re_ds_offset_t offset,
            l4re_ds_flags_t flags,
            l4re_ds_map_addr_t local_addr,
            l4re_ds_map_addr_t min_addr,
            l4re_ds_map_addr_t max_addr) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> x(ds);
  return x->map(offset, L4Re::Dataspace::Flags(flags), local_addr, min_addr, max_addr);
}

int
l4re_ds_map_region(l4re_ds_t ds, l4re_ds_offset_t offset,
                   l4re_ds_flags_t flags,
                   l4re_ds_map_addr_t min_addr,
                   l4re_ds_map_addr_t max_addr) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> x(ds);
  return x->map_region(offset, L4Re::Dataspace::Flags(flags), min_addr, max_addr);
}

long
l4re_ds_clear(l4re_ds_t ds, l4re_ds_offset_t offset, l4re_ds_size_t size) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> x(ds);
  return x->clear(offset, size);
}

long
l4re_ds_allocate(l4re_ds_t ds,
                 l4re_ds_offset_t offset, l4re_ds_size_t size) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> x(ds);
  return x->allocate(offset, size);
}

int
l4re_ds_copy_in(l4re_ds_t ds, l4re_ds_offset_t dst_offs,
                l4re_ds_t src, l4re_ds_offset_t src_offs,
                l4re_ds_size_t size) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> x(ds);
  auto srcds = L4::Ipc::Cap<L4Re::Dataspace>::from_ci(src);
  return x->copy_in(dst_offs, srcds, src_offs, size);
}

l4re_ds_size_t
l4re_ds_size(l4re_ds_t ds) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> x(ds);
  return x->size();
}

int
l4re_ds_info(l4re_ds_t ds, l4re_ds_stats_t *stats) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> x(ds);
  return x->info(reinterpret_cast<L4Re::Dataspace::Stats *>(stats));
}

l4re_ds_flags_t
l4re_ds_flags(l4re_ds_t ds) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> x(ds);
  return x->flags().raw;
}

int
l4re_ds_map_info(l4re_ds_t ds,
                 l4_addr_t *start_addr, l4_addr_t *end_addr) L4_NOTHROW
{
  L4::Cap<L4Re::Dataspace> x(ds);
  return x->map_info(start_addr, end_addr);
}
