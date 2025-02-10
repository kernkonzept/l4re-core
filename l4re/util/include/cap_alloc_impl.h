// -*- Mode: C++ -*-
// vim:ft=cpp
/**
 * \file
 * \brief  Capability allocator implementation
 */
/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <l4/bid_config.h>
#include <l4/re/cap_alloc>

#if defined(CONFIG_L4RE_BITMAP_CAP_ALLOC)

#include <l4/re/util/bitmap_cap_alloc>

namespace L4Re { namespace Util {

using _Cap_alloc_impl = Cap_alloc_base;

}}

#elif defined(CONFIG_L4RE_COUNTING_CAP_ALLOC)

#include <l4/re/util/counting_cap_alloc>
#include <l4/re/util/debug>

namespace L4Re { namespace Util {

// RISC-V does not natively support subword atomics, such as __atomic_load_1.
// The RISC-V gcc developers have decided to emulate these via libatomic, which
// is automatically linked against.
#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1) || defined(ARCH_arm) || defined(ARCH_riscv)
using _Cap_alloc_impl
  = Counting_cap_alloc<L4Re::Util::Counter_atomic<unsigned char>,
                       L4Re::Util::Dbg>;
#elif defined(ARCH_sparc)
using _Cap_alloc_impl
  = Counting_cap_alloc<L4Re::Util::Counter<unsigned char>,
                       L4Re::Util::Dbg>;
#warning "Thread-safe capability allocator not available!"
#else
#error "Unsupported platform"
#endif

}}

#else
#error "No supported capability allocator selected"
#endif

namespace L4Re { namespace Util {

/**
 * Adapter to expose the cap allocator implementation as L4Re::Cap_alloc
 * compatible class.
 *
 * Not intended to be used in application code.
 */
class _Cap_alloc final : public L4Re::Cap_alloc, private _Cap_alloc_impl
{
public:
  template <unsigned COUNT>
  using Storage = _Cap_alloc_impl::Storage<COUNT>;

  using _Cap_alloc_impl::_Cap_alloc_impl; // Expose underlying constructor
  void operator delete(void *) {} // Prevent global operator delete reference

  L4::Cap<void> alloc() noexcept override
  { return _Cap_alloc_impl::alloc(); }

  template< typename T >
  L4::Cap<T> alloc() noexcept
  { return L4::cap_cast<T>(alloc()); }

  void take(L4::Cap<void> cap) noexcept override
  { _Cap_alloc_impl::take(cap); }

  void free(L4::Cap<void> cap, l4_cap_idx_t task = L4_INVALID_CAP,
            unsigned unmap_flags = L4_FP_ALL_SPACES) noexcept override
  { _Cap_alloc_impl::free(cap, task, unmap_flags); }

  bool release(L4::Cap<void> cap, l4_cap_idx_t task,
               unsigned unmap_flags) noexcept override
  { return _Cap_alloc_impl::release(cap, task, unmap_flags); }

  using _Cap_alloc_impl::last;
};

}}
