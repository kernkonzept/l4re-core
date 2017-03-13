/*
 * Copyright (C) 2016 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Tests for Cache-flush API
 */

#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>
#include <l4/sys/cache.h>

#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>


static L4Re::Env const * const env = L4Re::Env::env();

template <bool MEM>
struct Test_range
{
  enum { Mem_available = MEM };

  void check_result(int result, char const *comment)
  {
    if (MEM)
      EXPECT_L4OK(result) << comment;
    else
      EXPECT_TRUE(result == L4_EOK || result == -L4_EFAULT) << comment;
  }

  l4_addr_t start;
  l4_addr_t end;
};

// Test with memory already mapped.
class MappedDataspace : public Test_range<true>
{
public:
  MappedDataspace()
  {
    _ds = L4Re::chkcap(L4Re::Util::make_auto_del_cap<L4Re::Dataspace>());
    L4Re::chksys(env->mem_alloc()->alloc(L4_PAGESIZE, _ds.get()));
    L4Re::chksys(env->rm()->attach(&_region, L4_PAGESIZE,
                                   L4Re::Rm::Search_addr | L4Re::Rm::Eager_map,
                                   _ds.get(), 0));
    start = reinterpret_cast<l4_addr_t>(_region.get()) + 8;
    end = start + 555;
  }

private:
  L4Re::Util::Auto_del_cap<L4Re::Dataspace>::Cap _ds;
  L4Re::Rm::Auto_region<void *> _region;
};

// Test with memory that needs to be mapped before flushing caches works.
class UnmappedDataspace : public Test_range<true>
{
public:
  UnmappedDataspace()
  {
    _ds = L4Re::chkcap(L4Re::Util::make_auto_del_cap<L4Re::Dataspace>());
    L4Re::chksys(env->mem_alloc()->alloc(L4_PAGESIZE, _ds.get()));
    L4Re::chksys(env->rm()->attach(&_region, L4_PAGESIZE,
                                   L4Re::Rm::Search_addr, _ds.get(), 0));
    start = reinterpret_cast<l4_addr_t>(_region.get()) + 8;
    end = start + 555;
  }

private:
  L4Re::Util::Auto_del_cap<L4Re::Dataspace>::Cap _ds;
  L4Re::Rm::Auto_region<void *> _region;
};

// Test with memory where pagefaults cannot be resolved.
struct ReservedRange : Test_range<false>
{
  ReservedRange()
  {
    L4Re::chksys(env->rm()->reserve_area(&start, L4_PAGESIZE,
                                         L4Re::Rm::Search_addr));
    end = start + 456;
  }

  ~ReservedRange()
  { L4Re::chksys(env->rm()->free_area(start)); }
};


template <typename T>
class CacheTest : public testing::Test {};

typedef testing::Types<MappedDataspace,
                       UnmappedDataspace,
                       ReservedRange> MemoryTypes;
TYPED_TEST_CASE(CacheTest, MemoryTypes);

TYPED_TEST(CacheTest, CleanData)
{
  TypeParam obj;
  obj.check_result(l4_cache_clean_data(obj.start, obj.end),
                   "Clean data cache in test region");
}

TYPED_TEST(CacheTest, FlushData)
{
  TypeParam obj;
  obj.check_result(l4_cache_flush_data(obj.start, obj.end),
                   "Flush data cache in test region");
}

TYPED_TEST(CacheTest, InvData)
{
  TypeParam obj;
  obj.check_result(l4_cache_inv_data(obj.start, obj.end),
                   "Invalidate data cache in test region");
}

TYPED_TEST(CacheTest, CoherentCache)
{
  if (!TypeParam::Mem_available)
    return;

  TypeParam obj;
  obj.check_result(l4_cache_coherent(obj.start, obj.end),
                   "Flush instruction cache in test region");
}

TYPED_TEST(CacheTest, DmaCoherentData)
{
  TypeParam obj;
  obj.check_result(l4_cache_dma_coherent(obj.start, obj.end),
                   "Make cache coherent with external memory in test region");
}

#ifdef L4RE_TEST_HACK_APIS

TYPED_TEST(CacheTest, DmaCoherentFull)
{
  TypeParam obj;
  obj.check_result(l4_cache_dma_coherent_full(),
                   "Make entire memory cache coherent");
}

#endif
