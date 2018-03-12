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
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#pragma once

//#define L4RE_STATIC_CAP_ALLOC
#if defined(L4RE_STATIC_CAP_ALLOC)

#include <l4/re/util/bitmap_cap_alloc>

namespace L4Re { namespace Util {

typedef Cap_alloc_base _Cap_alloc;

}}

#else
#include <l4/re/util/counting_cap_alloc>

namespace L4Re { namespace Util {

typedef Counting_cap_alloc<L4Re::Util::Counter<unsigned char> > _Cap_alloc;

}}
#endif


