/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/bid_config.h>
#include <l4/cxx/iostream>
#include <l4/libumalloc/umalloc.h>

extern char __heap_start[];
extern char __heap_end[];
static char *heap_pos = __heap_start;

// The default heap size is 64KiB. We don't expect any user to override
// DEFAULT_HEAP_SIZE_l4re with orders of magnitude more. So allocate just a
// single area. Fortunately, libumalloc can cope with any granularity.
size_t umalloc_area_granularity = __heap_end - __heap_start;

void *umalloc_area_create(size_t area_size) noexcept
{
  if (static_cast<size_t>(__heap_end - heap_pos) < area_size)
    {
      L4::cout << "l4re_itas: ERROR: static heap exhausted\n";
      return nullptr;
    }

  void *ret = heap_pos;
  heap_pos += area_size;
  return ret;
}
