/*
 * Copyright (C) 2008-2009, 2026 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *            Alexander Warg <warg@os.inf.tu-dresden.de>
 *            Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include "init_memory.h"

#include "debug.h"
#include "globals.h"
#include "mem_region_list.h"
#include "page_alloc.h"
#include "pages.h"
#include "ram_ranges.h"

#include <l4/cxx/minmax>
#include <l4/sigma0/sigma0.h>
#include <l4/sys/kip>
#include <l4/util/kip.h>
#include <l4/util/printf_helpers.h>

#include <cstdlib>

static Dbg info(Dbg::Info);
static Dbg boot(Dbg::Boot);

l4_size_t Moe::Phys_limit::avail_ram;

namespace
{

using Free_map = Moe::Mem_region_list<Moe::Mem_range, 100>;

/// Map all available memory from sigma0 and populate the free map with it.
void map_all_avail_memory(Free_map &free_map)
{
  l4_addr_t addr;
  for (unsigned order = Moe::Max_phys_page_order; order >= L4_LOG2_PAGESIZE; --order)
    {
      while (!l4sigma0_map_anypage(Sigma0_cap, 0, L4_WHOLE_ADDRESS_SPACE,
                                   &addr, order))
        {
          unsigned long size = 1UL << order;

          // Moe identity maps physical memory into its virtual address space.
          // To not clash with nullptr, physical memory at address zero must not
          // be used.
          if (addr == 0)
            {
              addr = L4_PAGESIZE;
              size -= L4_PAGESIZE;
              if (!size)
                continue;
            }

          Moe::Mem_range range{addr, addr + size - 1};
          if (free_map.add(range))
            continue; // success

          // TODO: Initial stack allocated free map is exhausted.
          //       Need to allocate temporary free map from the free memory we already discovered.
          //       Must be a region specified as untyped in Moe::root_factory_config.
          Err(Err::Normal).printf("free map exhausted\n");
          // For now just ignore the remaining memory.
          break;
        }
      if (free_map.full())
        break;
    }

  info.printf("Free map:-----------------------\n");
  free_map.dump(info);
}

/**
 * Holes smaller than this are not worth a bank of their own.
 *
 * A hole costs roughly two bytes of buddy metadata per 4 KiB (L4_PAGESIZE), so
 * accepting a 64 MiB hole costs about 32 KiB of wasted metadata memory. That is
 * preferable to dealing with a lot of banks, and negligible compared with the
 * holes this is here to avoid, which are gigabytes wide.
 *
 * The same applies for the reference counters, where a hole costs four bytes
 * per 4 KiB (L4_PAGESIZE), so accepting a 64 MiB hole costs about 64 KiB of
 * wasted wasted reference counter memory.
 */
constexpr l4_size_t Bank_split_hole = 64UL << 20;

/**
 * Add [start, last] to the banks, merging where a split would not pay.
 *
 * The regions arrive in ascending order, so a region either extends the last
 * bank or starts a new one.
 *
 * \return The new number of banks.
 */
void
add_bank(Single_page_alloc_base::Ram_bank_list &banks, Moe::Mem_range new_range)
{
  auto hole_before = [](Moe::Mem_range const &base, Moe::Mem_range const &next)
    { return next.start - base.end - 1; };

  if (banks.empty())
    {
      banks.push_back(new_range);
      return;
    }

  Moe::Mem_range &last_bank = banks.last();
  if (new_range.start <= last_bank.end)
    {
      // Not expected from a sorted, disjoint free map, but never let the end
      // move backwards if it happens.
      last_bank.end = cxx::max(last_bank.end, new_range.end);
      return;
    }

  if (hole_before(last_bank, new_range) <= Bank_split_hole)
    {
      last_bank.end = new_range.end;
      return;
    }

  if (!banks.full())
    {
      banks.push_back(new_range);
      return;
    }

  // Out of banks. Give up the smallest hole, since that is the one whose
  // metadata costs least - possibly the one in front of the new region, in
  // which case the new region simply joins the last bank.
  unsigned victim = banks.len() - 1;
  l4_size_t smallest = hole_before(banks.last(), new_range);
  for (unsigned i = 0; i + 1 < banks.len(); ++i)
    {
      auto hole = hole_before(banks[i], banks[i + 1]);
      if (hole < smallest)
        {
          smallest = hole;
          victim = i;
        }
    }

  if (victim == banks.len() - 1)
    {
      banks[victim].end = new_range.end;
      return;
    }

  banks[victim + 1].start = banks[victim].start;
  banks.del(victim);
  banks.push_back(new_range);
}

/// Allocate metadata for allocator and initialize the allocator.
void init_page_alloc(Free_map const &free_map)
{
  // Split the memory into banks so that the buddy allocators do not pay for
  // the holes between them, and size their metadata.
  Single_page_alloc_base::Ram_bank_list banks;
  for (auto const &region : free_map)
    add_bank(banks, region);

  info.printf("Memory banks:-------------------\n");
  banks.dump(info);

  // Now find the optimal place for buddy allocator free bits.
  unsigned long metadata_bytes =
    Single_page_alloc_base::_metadata_bytes(banks);
  info.printf("Allocator free map:-------------\n");
  info.printf("Required bytes: %#lx\n", metadata_bytes);

  Moe::Mem_range metadata_region;
  // Find smallest region to avoid destroying precious large aligned ram.
  for (auto const &free_region : free_map)
    {
      if (free_region.size() < metadata_bytes)
        continue;

      // Only parts of the region in untyped memory are eligible.
      for (auto const &untyped_region : Moe::root_factory_config.untyped_regions())
        {
          auto intersect = free_region.intersect(untyped_region.range);
          if (!intersect.valid() || intersect.size() < metadata_bytes)
            continue;

          if (metadata_region.valid() && metadata_region.size() <= intersect.size())
            continue;

          metadata_region = intersect;
        }
    }

  if (!metadata_region.valid())
    {
      Err(Err::Fatal).printf("no suitable region for alloc metadata\n");
      exit(128);
    }

  // We allocate the metadata at the start of the selected region.
  metadata_region.end = metadata_region.start + metadata_bytes - 1;

  info.printf("Allocator metadata region:\n");
  metadata_region.dump(info);

  Single_page_alloc_base::_init(
    banks, reinterpret_cast<unsigned char *>(metadata_region.start),
    metadata_bytes);

  // Finally add the memory to the allocator
  for (auto const &region : free_map)
    {
      // Carve out metadata region.
      if (region.contains(metadata_region))
        {
          if (metadata_region.start > region.start)
            Single_page_alloc_base::_add_mem(
              reinterpret_cast<void *>(region.start),
              metadata_region.start - region.start);

          if (metadata_region.end < region.end)
            Single_page_alloc_base::_add_mem(
              reinterpret_cast<void *>(metadata_region.end + 1),
              region.end - metadata_region.end);
        }
      else
        Single_page_alloc_base::_add_mem(reinterpret_cast<void *>(region.start),
                                         region.size());
    }

  Moe::Phys_limit::avail_ram = Single_page_alloc_base::_avail();
  char str[64];
  l4util_human_readable_size(str, sizeof(str), Single_page_alloc_base::_avail());
  info.printf("found %s RAM in the area %08lx..%08lx\n", str, free_map[0].start,
              free_map.last().end);
}

#ifdef CONFIG_MMU
/// Allocate and clear the reference counters for the collected memory ranges.
void setup_page_banks(Free_map const &free_map)
{
  Moe::Ram_ranges ram;

  // Note the memory only after it has been handed to the page allocator, the
  // ranges are stored in memory from that allocator.
  for (auto const &region : free_map)
    ram.add(region);

  // boot modules are handed out as dataspaces as well and thus need
  // reference counters too
  for (auto const &md: L4::Kip::Mem_desc::all(kip()))
    {
      if (md.is_virtual())
        continue;

      L4::Kip::Mem_desc::Mem_type type = md.type();
      switch (type)
        {
        case L4::Kip::Mem_desc::Bootloader:
          ram.add(Moe::Mem_range(md.start(), md.start() + md.size() - 1));
          break;
        case L4::Kip::Mem_desc::Conventional:
        case L4::Kip::Mem_desc::Reserved:
        case L4::Kip::Mem_desc::Dedicated:
        case L4::Kip::Mem_desc::Arch:
        case L4::Kip::Mem_desc::Shared:
        default:
          break;
        }
    }

  // All memory is known now, small holes are not worth a memory bank.
  ram.merge_small_holes(Bank_split_hole);

  assert(ram.count() > 0);

  auto *banks = static_cast<Moe::Pages::Bank *>(
                  Single_page_alloc_base::_alloc(
                    ram.count() * sizeof(Moe::Pages::Bank), L4_PAGESIZE));

  l4_size_t total = ram.count() * sizeof(Moe::Pages::Bank);
  unsigned i = 0;
  for (auto const &ram_range: ram)
    {
      auto &bank = banks[i++];
      bank.range = ram_range;

      l4_size_t sz = (bank.range.size() >> L4_PAGESHIFT) * sizeof(*bank.ref_counts);
      bank.ref_counts = static_cast<decltype(bank.ref_counts)>(
                       Single_page_alloc_base::_alloc(sz, L4_PAGESIZE));
      memset(bank.ref_counts, 0, sz);
      total += sz;

      boot.printf("memory bank %u: %08lx..%08lx, page array @%p (%zu bytes)\n",
                  i, bank.range.start, bank.range.end, bank.ref_counts, sz);
    }

  // Only publish the banks after all of them are ready for use.
  Moe::Pages::banks     = banks;
  Moe::Pages::num_banks = ram.count();

  char str[64];
  l4util_human_readable_size(str, sizeof(str), total);
  info.printf("allocated %s for the page arrays of %u memory bank(s)\n",
              str, Moe::Pages::num_banks);
}
#endif

} // namespace

void Moe::find_memory()
{
  Single_page_alloc_base::can_free =
    l4util_kip_kernel_has_feature(kip(), "mapdb");
  if (!Single_page_alloc_base::can_free)
    info.printf("Fiasco mapdb not available! Memory cannot be given back!\n");

  // List of free regions (allocated on stack).
  Free_map free_map;
  map_all_avail_memory(free_map);

  init_page_alloc(free_map);

#ifdef CONFIG_MMU
  setup_page_banks(free_map);
#endif
}


l4_addr_t Moe::Virt_limit::start;
l4_addr_t Moe::Virt_limit::end;

void Moe::init_virt_limits()
{
  for (auto const &m: L4::Kip::Mem_desc::all(kip()))
    {
      if (m.type() != L4::Kip::Mem_desc::Conventional || !m.is_virtual())
        continue;

      Moe::Virt_limit::start = m.start();
      Moe::Virt_limit::end = m.end();
    }

  info.printf("virtual user address space [%lx-%lx]\n",
              Moe::Virt_limit::start,
              Moe::Virt_limit::end);
}
