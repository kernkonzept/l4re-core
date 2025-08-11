/*
 * Copyright (C) 2008-2009, 2025 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *            Alexander Warg <warg@os.inf.tu-dresden.de>
 *            Martin Decky <martin.decky@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <cstddef>
#include <l4/sys/consts.h>
#include <l4/libumalloc/umalloc.h>
#include "page_alloc.h"

size_t umalloc_area_granularity = L4_PAGESIZE;

void *umalloc_area_create(size_t area_size) noexcept
{
  return Single_page_alloc_base::_alloc(Single_page_alloc_base::nothrow,
                                        l4_round_page(area_size), L4_PAGESIZE);
}
