/**
 * \file
 * \brief Atomic template
 */
/*
 * (c) 2004-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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

#ifndef L4_CXX_ATOMIC_H__
#define L4_CXX_ATOMIC_H__

#include <l4/util/atomic.h>

extern "C" void  ____error_compare_and_swap_does_not_support_3_bytes____();
extern "C" void  ____error_compare_and_swap_does_not_support_more_than_4_bytes____();

namespace L4
{
  template< typename X >
  inline int compare_and_swap(X volatile *dst, X old_val, X new_val)
  {
    switch (sizeof(X))
      {
      case 1:
	return l4util_cmpxchg8((l4_uint8_t volatile*)dst, old_val, new_val);
      case 2:
	return l4util_cmpxchg16((l4_uint16_t volatile *)dst, old_val, new_val);
      case 3: ____error_compare_and_swap_does_not_support_3_bytes____();
      case 4:
	return l4util_cmpxchg32((l4_uint32_t volatile*)dst, old_val, new_val);
      default:
        ____error_compare_and_swap_does_not_support_more_than_4_bytes____();
      }
    return 0;
  }
};

#endif // L4_CXX_ATOMIC_H__
