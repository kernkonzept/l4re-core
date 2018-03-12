/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/compiler.h>
#include <l4/re/rm>
#include <l4/re/dataspace>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <errno.h>
#include <l4/l4re_vfs/backend>
#include <bits/l4-malloc.h>
#include "redirect.h"

#define L4B_REDIRECT(ret, func, ptlist, plist) \
  ret func ptlist L4_NOTHROW                                    \
  {                          \
    ret r = L4B(func plist); \
    POST();                  \
  }

void *mmap2(void *addr, size_t length, int prot, int flags,
            int fd, off_t pgoffset) L4_NOTHROW;
void *mmap2(void *addr, size_t length, int prot, int flags,
            int fd, off_t pgoffset) L4_NOTHROW
{
  void *resptr;
  int r = L4B(mmap2(addr, length, prot, flags, fd, pgoffset, &resptr));
  if (r < 0)
    {
      errno = -r;
      return MAP_FAILED;
    }

  return resptr;
}


/* Other versions of mmap */
void *mmap64(void *addr, size_t length, int prot, int flags,
             int fd, off64_t offset) L4_NOTHROW
{
  if (offset & ~L4_PAGEMASK)
    {
      errno = EINVAL;
      return MAP_FAILED;
    }
  return mmap2(addr, length, prot, flags, fd, offset >> 12);
}

void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset) L4_NOTHROW
{
  return mmap64(addr, length, prot, flags, fd, offset);
}

L4B_REDIRECT_2(int, munmap, void*, size_t)
L4B_REDIRECT_3(int, mprotect, void *, size_t, int);
L4B_REDIRECT_3(int, madvise, void *, size_t, int);
L4B_REDIRECT_3(int, msync, void *, size_t, int);

void *mremap(void *old_addr, size_t old_size, size_t new_size,
             int flags, ...) L4_NOTHROW
{
  void *resptr;
  if (flags & MREMAP_FIXED)
    {
      va_list a;
      va_start(a, flags);
      resptr = va_arg(a, void *);
      va_end(a);
    }

  int r = L4B(mremap(old_addr, old_size, new_size, flags, &resptr));
  if (r < 0)
    {
      errno = -r;
      return MAP_FAILED;
    }

  return resptr;
}

static void *current_morecore_end;

void *uclibc_morecore(long bytes)
{
  // calling morecore with 0 size is done by the malloc implementation
  // to check for the amount of memory it got from the last call to morecore
  // With a negative value, 'free' wants to return memory, we do not support
  // that here.
  if (bytes <= 0)
    return current_morecore_end;

  size_t s = l4_round_page(bytes);
  void *b = mmap2(0, s, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
  if (L4_UNLIKELY(b == MAP_FAILED))
    return b;

  current_morecore_end = static_cast<char *>(b) + s;
  return b;
}

