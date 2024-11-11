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

#if defined(CONFIG_L4RE_BITMAP_CAP_ALLOC)

#include <l4/re/util/bitmap_cap_alloc>

namespace L4Re { namespace Util {

typedef Cap_alloc_base _Cap_alloc;

}}

#elif defined(CONFIG_L4RE_COUNTING_CAP_ALLOC)

#include <l4/re/util/counting_cap_alloc>
#include <l4/re/util/debug>

namespace L4Re { namespace Util {

// RISC-V does not natively support subword atomics, such as __atomic_load_1.
// The RISC-V gcc developers have decided to emulate these via libatomic, which
// is automatically linked against.
#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1) || defined(ARCH_arm) || defined(ARCH_riscv)
typedef Counting_cap_alloc<L4Re::Util::Counter_atomic<unsigned char>,
                           L4Re::Util::Dbg > _Cap_alloc;
#elif defined(ARCH_sparc)
typedef Counting_cap_alloc<L4Re::Util::Counter<unsigned char>,
                           L4Re::Util::Dbg > _Cap_alloc;
#warning "Thread-safe capability allocator not available!"
#else
#error "Unsupported platform"
#endif

}}

#else
#error "No supported capability allocator selected"
#endif
