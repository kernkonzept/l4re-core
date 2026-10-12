/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/util/util.h>

#include <l4/cxx/buddy_alloc>
#include <l4/cxx/iostream>
#include <l4/cxx/minmax>
#include <l4/cxx/exceptions>
#include "page_alloc.h"
#include "debug.h"

#if 1
enum { page_alloc_debug = 0 };
#else
unsigned page_alloc_debug = 0;
#endif

using Page_alloc = cxx::Buddy_alloc;

namespace {

/// One buddy allocator plus the span it covers.
struct Bank
{
  Page_alloc alloc;
  Moe::Mem_range range;
};

/**
 * The allocator banks, in ascending address order.
 *
 * Statically sized because they are set up before there is any allocator to
 * allocate them from.
 */
Bank banks[Single_page_alloc_base::Max_ram_banks];
unsigned num_banks;

/// The bank `p` belongs to, or nullptr if it belongs to none.
Bank *bank_of(void const *p, size_t size)
{
  l4_addr_t addr = reinterpret_cast<l4_addr_t>(p);
  [[maybe_unused]] l4_addr_t end_addr = addr + size - 1; // inclusive
  assert(end_addr >= addr); // overflow

  for (unsigned i = 0; i < num_banks; ++i)
    if (banks[i].range.contains_addr(addr))
      {
        assert(banks[i].range.contains_addr(end_addr));
        return &banks[i];
      }

  return nullptr;
}

}

bool Single_page_alloc_base::can_free = false;
Single_page_alloc_base::Config Single_page_alloc_base::default_mem_cfg;

Single_page_alloc_base::Single_page_alloc_base()
{}

size_t
Single_page_alloc_base::_avail()
{
  size_t avail = 0;

  for (unsigned i = 0; i < num_banks; ++i)
    avail += banks[i].alloc.avail();

  return avail;
}

void *
Single_page_alloc_base::_alloc_max(size_t min, size_t *max, size_t align,
                                   size_t granularity, Config cfg)
{
  void *ret = nullptr;
  // TODO: If there are multiple regions an alloc_max might allocate less memory
  //       than actually would have been possible in a following region. The
  //       same applies to the banks within a region.
  for (auto const &region : cfg.regions)
    {
      for (unsigned i = 0; i < num_banks; ++i)
        {
          Moe::Mem_range range = banks[i].range.intersect(region.range);
          if (!range.valid())
            continue;

          size_t alloc_size = *max;
          ret = banks[i].alloc.alloc_max(min, &alloc_size, align, granularity,
                                         range.start, range.end);
          if (ret)
            {
              *max = alloc_size;
              break; // successful allocation
            }
        }
    }

  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): alloc_max(" << *max << ") @" << ret << '\n';
  return ret;
}

void *
Single_page_alloc_base::_alloc(Nothrow, size_t size, size_t align, Config cfg)
{
  void *ret = nullptr;
  for (auto const &region : cfg.regions)
    {
      for (unsigned i = 0; i < num_banks; ++i)
        {
          Moe::Mem_range range = banks[i].range.intersect(region.range);
          if (!range.valid())
            continue;

          ret = banks[i].alloc.alloc(size, align, range.start, range.end);
          if (ret)
            break; // successful allocation
        }
    }

  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): alloc(" << size << ") @" << ret << '\n';
  return ret;
}

void
Single_page_alloc_base::_free(void *p, size_t size)
{
  if (can_free)
    _free_internal(p, size);
}

void
Single_page_alloc_base::_free_internal(void *p, size_t size)
{
  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): free(" << size << ") @" << p << '\n';

  if (Bank *b = bank_of(p, size))
    b->alloc.free(p, size);
}

void
Single_page_alloc_base::_add_mem(void *p, size_t size)
{
  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): add_mem(" << size << ") @" << p << '\n';

  if (Bank *b = bank_of(p, size))
    b->alloc.add_mem(p, size);
  else
    // Must never happen, memory must always come from one of the banks.
    Err(Err::Normal).printf("no memory bank for %p+%zx, memory dropped\n",
                            p, size);
}

/// Metadata of one bank, rounded up so the next bank's starts on a page.
static size_t bank_metadata_bytes(Single_page_alloc_base::Ram_bank const &b)
{
  return l4_round_page(Page_alloc::metadata_bytes(b.start, b.end));
}

size_t
Single_page_alloc_base::_metadata_bytes(Ram_bank_list const &ram_banks)
{
  size_t total = 0;

  for (auto const &ram_bank : ram_banks)
    total += bank_metadata_bytes(ram_bank);

  if (page_alloc_debug)
    L4::cout << "pa(" << __builtin_return_address(0) << "): metadata_bytes("
             << ram_banks.len() << " ram_banks) = " << total << '\n';

  return total;
}

void
Single_page_alloc_base::_init(Ram_bank_list const &ram_banks,
                              unsigned char *metadata_addr, size_t metadata_size)
{
  unsigned char *cur_meta = metadata_addr;
  for (unsigned i = 0; i < ram_banks.len(); ++i)
    {
      auto const &ram_bank = ram_banks[i];
      size_t bank_meta_size = bank_metadata_bytes(ram_bank);
      assert(cur_meta - metadata_addr + bank_meta_size <= metadata_size);

      if (page_alloc_debug)
        L4::cout << "pa: bank " << i << " init(" << ram_bank.start << ", "
                 << ram_bank.end << ", " << cur_meta << ", " << bank_meta_size
                 << ")" << '\n';

      banks[i].range = ram_bank;
      banks[i].alloc.init(ram_bank.start, ram_bank.end, cur_meta, bank_meta_size);

      cur_meta += bank_meta_size;
    }

  // Publish only once every bank can be used.
  num_banks = ram_banks.len();
}

#ifndef NDEBUG
void
Single_page_alloc_base::_dump_free(Dbg &dbg)
{
  for (unsigned i = 0; i < num_banks; ++i)
    {
      dbg.printf("bank %u: %08lx - %08lx\n", i, banks[i].range.start,
                 banks[i].range.end);
      banks[i].alloc.dump_free(dbg);
    }
}
#endif
