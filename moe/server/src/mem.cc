/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#define _GNU_SOURCE 1
#include <bits/l4-malloc.h>
#include <sys/mman.h>
#include <errno.h>

#include "page_alloc.h"

#include <l4/sys/types.h>

#include <l4/cxx/iostream>

static void *current_morecore_end;

void *uclibc_morecore(long bytes)
{
  // Calling morecore with 0 size is done by the malloc/free implementation
  // to check for the amount of memory it got from the last call to
  // morecore.
  // With a negative value, 'free' wants to return memory, we do not support
  // that here.
  if (bytes <= 0)
    return current_morecore_end;

  size_t s = l4_round_page(bytes);
  void *b = Single_page_alloc_base::_alloc(Single_page_alloc_base::nothrow,
                                           s, L4_PAGESIZE);
  if (L4_UNLIKELY(!b))
    return MAP_FAILED;

  current_morecore_end = static_cast<char *>(b) + s;
  return b;
}
