/**
 * \file
 * \brief Atomic template
 */
/*
 * (c) 2004-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

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
}
