/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#pragma once

/*
 * Various helpers for Moe tests.
 */

#include <cstring>
#include <l4/re/env>
#include <l4/re/dataspace>
#include <l4/re/namespace>
#include <l4/re/dma_space>
#include <l4/sys/vcon>
#include <l4/util/splitlog2.h>
#include <l4/sys/factory>
#include <l4/sys/scheduler>
#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/util/shared_cap>

static L4Re::Env const * const env = L4Re::Env::env();

template <typename T>
L4Re::Util::Unique_del_cap<T>
make_unique_del_cap()
{
  return L4Re::chkcap(L4Re::Util::make_unique_del_cap<T>());
}

template <typename T>
L4Re::Util::Unique_cap<T>
make_unique_cap()
{
  return L4Re::chkcap(L4Re::Util::make_unique_cap<T>());
}

template <typename T>
typename L4Re::Util::Ref_cap<T>::Cap
make_ref_cap()
{
  return L4Re::chkcap(L4Re::Util::make_ref_cap<T>());
}

template <typename T>
L4Re::Util::Shared_cap<T>
make_shared_cap()
{
  return L4Re::chkcap(L4Re::Util::make_shared_cap<T>());
}


inline
L4Re::Util::Unique_del_cap<L4Re::Dataspace>
create_ds(unsigned long flags = 0, unsigned long size = L4_PAGESIZE,
          L4::Cap<L4Re::Mem_alloc> fab = env->mem_alloc())
{
  auto ds = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dataspace>());
  L4Re::chksys(fab->alloc(size, ds.get(), flags));
  return ds;
}

inline
L4Re::Util::Unique_del_cap<L4Re::Dataspace>
create_cont_ds(unsigned long size = L4_PAGESIZE,
               L4::Cap<L4Re::Mem_alloc> fab = env->mem_alloc())
{
  auto ds = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dataspace>());
  L4Re::chksys(fab->alloc(size, ds.get(), L4Re::Mem_alloc::Continuous));
  return ds;
}

inline
L4Re::Util::Unique_del_cap<L4::Factory>
create_fab(unsigned long limit = 10 * L4_PAGESIZE,
           L4::Cap<L4::Factory> fab = env->user_factory())
{
  auto cap = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4::Factory>());
  L4Re::chksys(fab->create_factory(cap.get(), limit));
  return cap;
}

inline
L4Re::Util::Unique_del_cap<L4Re::Mem_alloc>
create_ma(unsigned long limit = 10 * L4_PAGESIZE,
          L4::Cap<L4::Factory> fab = env->user_factory())
{
  auto cap = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Mem_alloc>());
  L4Re::chksys(fab->create_factory(cap.get(), limit));
  return cap;
}

inline
L4Re::Util::Unique_del_cap<L4Re::Namespace>
create_ns(L4::Cap<L4::Factory> fab = env->user_factory())
{
  auto cap = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Namespace>());
  L4Re::chksys(fab->create(cap.get()));
  return cap;
}

inline
L4Re::Util::Unique_del_cap<L4Re::Dma_space>
create_dma(L4::Cap<L4::Factory> fab = env->user_factory())
{
  auto cap = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dma_space>());
  L4Re::chksys(fab->create(cap.get()));
  L4Re::chksys(cap->associate(L4::Ipc::Cap<L4::Task>(),
                              L4Re::Dma_space::Phys_space));
  return cap;
}

inline
L4Re::Util::Unique_del_cap<L4Re::Rm>
create_rm(L4::Cap<L4::Factory> fab = env->user_factory())
{
  auto cap = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Rm>());
  L4Re::chksys(fab->create(cap.get()));
  return cap;
}


// A Self-cleaning memory region with fencing pages at the beginning and
// the end that can be used to check for overmapping.
class Fenced_auto_area
{
public:
  Fenced_auto_area(unsigned long size,
                          unsigned flags = L4Re::Rm::Search_addr,
                          unsigned long fence_size = L4_PAGESIZE)
  : _size(2 * fence_size + l4_round_size(size, L4_PAGESHIFT)),
    _fence_size(fence_size)
  {
    L4Re::chksys(env->rm()->reserve_area(&_start, _size, flags));
    _front_region.reset((char *) _start, L4::Cap<L4Re::Rm>::Invalid);
    _back_region.reset((char *) _start + _size - fence_size,
                       L4::Cap<L4Re::Rm>::Invalid);

    _front_fence = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dataspace>());
    L4Re::chksys(env->mem_alloc()->alloc(fence_size, _front_fence.get(), 0));
    L4Re::chksys(env->rm()->attach(&_front_region, fence_size,
                                   L4Re::Rm::In_area, _front_fence.get()));
    memset(_front_region.get(), '{', fence_size);

    _back_fence = L4Re::chkcap(L4Re::Util::make_unique_del_cap<L4Re::Dataspace>());
    L4Re::chksys(env->mem_alloc()->alloc(fence_size, _back_fence.get(), 0));
    L4Re::chksys(env->rm()->attach(&_back_region, fence_size,
                                   L4Re::Rm::In_area, _back_fence.get()));
    memset(_back_region.get(), '}', fence_size);
  }

  /// Destructor. Unmaps the area and then frees the region.
  virtual ~Fenced_auto_area()
  {
    l4util_splitlog2_hdl(_start, _start + _size - 1, unmap_chunk);
    env->rm()->free_area(_start);
  }

  static long unmap_chunk(l4_addr_t s, l4_addr_t, int log2size)
  {
    env->task()->unmap(l4_fpage(s, log2size, L4_FPAGE_RWX), L4_FP_ALL_SPACES);
    return 0;
  }

  bool check_fence() const
  {
    for (unsigned i = 0; i < _fence_size; ++i)
      {
        if (_front_region.get()[i] != '{')
          return false;
        if (_back_region.get()[i] != '}')
          return false;
      }

    return true;
  }

  l4_addr_t start() const
  { return _start + _fence_size; }

  l4_addr_t end() const
  { return _start + _size - _fence_size; }

  template <typename T>
  T *data() const
  { return reinterpret_cast<T *>(_start + _fence_size); }

private:
  l4_addr_t _start;
  unsigned long _size;
  unsigned long _fence_size;
  L4Re::Rm::Unique_region<char *> _front_region;
  L4Re::Rm::Unique_region<char *> _back_region;
  L4Re::Util::Unique_del_cap<L4Re::Dataspace> _front_fence;
  L4Re::Util::Unique_del_cap<L4Re::Dataspace> _back_fence;
};

