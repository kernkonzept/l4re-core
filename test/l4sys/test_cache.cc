/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *            Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Tests for Cache-flush API
 *
 * Tests in this file only test that the API return the expected results.
 * They do not check that the expected function was actually executed.
 */

#include <l4/re/env>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/sys/cache.h>
#include <l4/sys/exception>
#include <l4/sys/cxx/ipc_epiface>

#include <l4/atkins/tap/main>
#include <l4/atkins/l4_assert>
#include <l4/atkins/fixtures/epiface_provider>
#include <l4/atkins/debug>

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

/**
 * Fixture for cache tests against memory that has already been
 * mapped to the user.
 */
class MappedDataspace : public Test_range<true>
{
public:
  MappedDataspace()
  {
    _ds = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dataspace>());
    L4Re::chksys(env->mem_alloc()->alloc(L4_PAGESIZE, _ds.get()));
    L4Re::chksys(env->rm()->attach(&_region, L4_PAGESIZE,
                                   L4Re::Rm::F::Search_addr | L4Re::Rm::F::Eager_map
                                   | L4Re::Rm::F::RWX,
                                   _ds.get(), 0));
    start = reinterpret_cast<l4_addr_t>(_region.get()) + 8;
    end = start + 555;
  }

private:
  L4Re::Util::Unique_del_cap<L4Re::Dataspace> _ds;
  L4Re::Rm::Unique_region<void *> _region;
};

/**
 * Fixture for cache tests against memory where a dataspace has been created
 * but not yet mapped to the user.
 */
class UnmappedDataspace : public Test_range<true>
{
public:
  UnmappedDataspace()
  {
    _ds = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dataspace>());
    L4Re::chksys(env->mem_alloc()->alloc(L4_PAGESIZE, _ds.get()));
    L4Re::chksys(env->rm()->attach(&_region, L4_PAGESIZE,
                                   L4Re::Rm::F::Search_addr | L4Re::Rm::F::RWX, _ds.get(), 0));
    start = reinterpret_cast<l4_addr_t>(_region.get()) + 8;
    end = start + 555;
  }

private:
  L4Re::Util::Unique_del_cap<L4Re::Dataspace> _ds;
  L4Re::Rm::Unique_region<void *> _region;
};

/**
 * Fixture for cache tests against memory regions where nothing is
 * allocated and therefore a page fault cannot be resolved.
 */
struct ReservedRange : Test_range<false>
{
  ReservedRange()
  {
    L4Re::chksys(env->rm()->reserve_area(&start, L4_PAGESIZE,
                                         L4Re::Rm::F::Search_addr));
    end = start + 456;
  }

  ~ReservedRange()
  { L4Re::chksys(env->rm()->free_area(start)); }
};

typedef testing::Types<MappedDataspace,
                       UnmappedDataspace,
                       ReservedRange> MemoryTypes;

/**
 * Extension of the exception handler interface to define a specially handled
 * region.
 */
struct Exc_handler_if : L4::Kobject_t<Exc_handler_if, L4::Exception>
{
  /**
   * Configure a special region to handle exceptions for.
   *
   * \param start  Start address of the region.
   * \param end    End address of the region (exclusive).
   */
  L4_INLINE_RPC(long, set_region, (l4_addr_t start, l4_addr_t end));

  using Rpcs = L4::Typeid::Rpcs<set_region_t>;
};

/**
 * Exception handler implementing the region extension interface.
 *
 * Page-fault exceptions for the defined region are ignored and the instruction
 * pointer is increased beyond the faulting instruction.
 */
class Exc_handler : public L4::Epiface_t<Exc_handler, Exc_handler_if>
{
  l4_addr_t _start = 0;
  l4_addr_t _end = 0;

  static Atkins::Dbg trace()
  {
    return Atkins::Dbg(Atkins::Dbg::Trace, "ExcHdlr");
  }

public:
  long op_set_region(L4::Typeid::Rights<Exc_handler_if> &,
                     l4_addr_t start, l4_addr_t end)
  {
    _start = start;
    _end = end;
    return L4_EOK;
  }

  long op_exception(L4::Exception::Rights, l4_exc_regs_t &regs,
                    L4::Ipc::Opt<L4::Ipc::Snd_fpage> &snd_fpage)
  {
    trace().printf("Received exception\n");

    if (l4_utcb_exc_is_pf(&regs))
      {
        l4_addr_t pfa = l4_utcb_exc_pfa(&regs);
        trace().printf("Exception is page fault at address 0x%lx\n", pfa);

        if((_start <= pfa) && (pfa < _end))
          {
            trace().printf("Page fault is in region range (start, pfa, end): "
                         "(0x%lx, 0x%lx, 0x%lx)\n",
                         _start, pfa, _end);

            l4_utcb_exc_pc_set(&regs, l4_utcb_exc_pc(&regs) + 4);

            return L4_EOK;
          }
        else
          trace().printf("Not a page fault in our region (0x%lx, 0x%lx)\n",
                       _start, _end);
      }
    else
      trace().printf("Not a page fault exception: Exc value 0x%lx\n",
                   l4_utcb_exc_typeval(&regs));

    // Exception is not a page fault and not in our defined region.
    L4::Cap<L4::Exception> own_exc_hdlr(L4_BASE_THREAD_CAP);
    auto ret =
      own_exc_hdlr->exception(&regs, L4::Ipc::Rcv_fpage(), snd_fpage.value());

    return ret.label();
  }
};

/**
 * Fixture for type-parametrized cache tests.
 *
 * In case no memory is mapped the cache operation shall proceed without error.
 * The implemented exception handler checks if an unresolved page fault is in
 * the expected range and steps over the faulting instruction.
 *
 * This is especially relevant for ARM64, where cache operations are executed
 * in user-space and page faults due to these operations are reflected back to
 * user-space.
 */
template <typename T>
class CacheTest : public Atkins::Fixture::Epiface_thread<Exc_handler>
{
public:
  CacheTest()
  {
    auto exc_hdlr = scap();
    exc_hdlr->set_region(obj.start, obj.end);

    L4::Thread::Attr attr;
    attr.exc_handler(exc_hdlr);
    L4Re::chksys(L4Re::Env::env()->main_thread()->control(attr),
                 "Set exception handler with region knowledge for the main "
                 "thread.");
  }

protected:
  T obj;
};

TYPED_TEST_CASE(CacheTest, MemoryTypes);

/**
 * A memory region can be cleaned in the data cache.
 *
 * \see l4_cache_clean_data
 */
TYPED_TEST(CacheTest, CleanData)
{
  this->obj.check_result(l4_cache_clean_data(this->obj.start, this->obj.end),
                         "Clean data cache in test region");
}

/**
 * A memory region can be flushed from the data cache.
 *
 * \see l4_cache_flush_data
 */
TYPED_TEST(CacheTest, FlushData)
{
  this->obj.check_result(l4_cache_flush_data(this->obj.start, this->obj.end),
                         "Flush data cache in test region");
}

/**
 * A memory region can be invalidated in the data cache.
 *
 * \see l4_cache_inv_data
 */
TYPED_TEST(CacheTest, InvData)
{
  this->obj.check_result(l4_cache_inv_data(this->obj.start, this->obj.end),
                         "Invalidate data cache in test region");
}

/**
 * A memory region can be made coherent between instruction and data cache.
 *
 * \see l4_cache_coherent
 */
TYPED_TEST(CacheTest, CoherentCache)
{
  if (!TypeParam::Mem_available)
    return;

  this->obj.check_result(l4_cache_coherent(this->obj.start, this->obj.end),
                         "Flush instruction cache in test region");
}

/**
 * A memory region can be made coherent for use with external access.
 *
 * \see l4_cache_dma_coherent
 */
TYPED_TEST(CacheTest, DmaCoherentData)
{
  this->obj
    .check_result(l4_cache_dma_coherent(this->obj.start, this->obj.end),
                  "Make cache coherent with external memory in test region");
}

#ifdef L4RE_TEST_HACK_APIS

/**
 * The entire data cache can be made coherent for use with external access.
 *
 * \see l4_cache_dma_coherent_full
 */
TYPED_TEST(CacheTest, DmaCoherentFull)
{
  this->obj.check_result(l4_cache_dma_coherent_full(),
                         "Make entire memory cache coherent");
}

#endif
