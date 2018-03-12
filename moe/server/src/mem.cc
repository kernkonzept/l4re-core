/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
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

void * mmap(void * /*start*/, size_t /*length*/, int /*prot*/, int /*flags*/, int /*fd*/, off_t /*offset*/) throw()
{
  L4::cout << "mmap() called: unimplemented!\n";
  errno = EINVAL;
  return MAP_FAILED;
}

int munmap(void * /*start*/, size_t  /*length*/) throw()
{
  L4::cout << "munmap() called: unimplemented!\n";
  errno = EINVAL;
  return -1;
}

void * mremap(void * /*old_address*/, size_t /*old_size*/, size_t /*new_size*/,
              int /*may_move*/, ...) throw()
{
  L4::cout << "mremap() called: unimplemented!\n";
  errno = EINVAL;
  return MAP_FAILED;
}
