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
  ret func ptlist noexcept(noexcept(func plist)) \
  {                          \
    ret r = L4B(func plist); \
    POST();                  \
  }

void *mmap2(void *addr, size_t length, int prot, int flags,
            int fd, off_t pgoffset) noexcept;
void *mmap2(void *addr, size_t length, int prot, int flags,
            int fd, off_t pgoffset) noexcept
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
             int fd, off64_t offset)
noexcept(noexcept(mmap64(addr, length, prot, flags, fd, offset)))
{
  if (offset & ~L4_PAGEMASK)
    {
      errno = EINVAL;
      return MAP_FAILED;
    }
  return mmap2(addr, length, prot, flags, fd, offset >> 12);
}

void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset)
noexcept(noexcept(mmap(addr, length, prot, flags, fd, offset)))
{
  return mmap64(addr, length, prot, flags, fd, offset);
}

L4B_REDIRECT_2(int, munmap, void*, size_t)
L4B_REDIRECT_3(int, mprotect, void *, size_t, int);
L4B_REDIRECT_3(int, madvise, void *, size_t, int);
L4B_REDIRECT_3(int, msync, void *, size_t, int);

void *mremap(void *old_addr, size_t old_size, size_t new_size,
             int flags, ...)
noexcept(noexcept(mremap(old_addr, old_size, new_size, flags)))
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

/* Use mmap from VFS unless the heap is provided by libc_be_static_heap. */
#ifndef CONFIG_BID_STATIC_HEAP
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
#endif

/* should be in mman.h -- will come with uclibc update */
int mlock2(const void *__addr, size_t len, unsigned int flags) noexcept;
int mlock2([[maybe_unused]] const void *__addr, [[maybe_unused]] size_t len,
           [[maybe_unused]] unsigned int flags) noexcept
{
  /* Implementation:
   * - Search for all regions between __addr and __addr+len
   *   - for each region:
   *     - page it in if flags != MLOCK_ONFAULT
   *     - tag range in dataspace that it is locked now
   *       - for that, add API to dataspace to lock+unlock pages
   *         (consider that dataspaces can be used from multiple tasks and
   *         that locking is a individual task-decision that needs to be
   *         considered for munlock, use a counter per page?)
   */

  return 0;
}

int mlock(const void *__addr, size_t __len) noexcept
{
  return mlock2(__addr, __len, 0);
}

int munlock([[maybe_unused]] const void *addr, [[maybe_unused]] size_t len) noexcept
{
  /* Implementation:
   * - Search for all regions between __addr and __addr+len
   *   - for each region:
   *     - unlock range in dataspace
   *       - for that, add API to dataspace to lock+unlock pages
   */
  return 0;
}

int mlockall([[maybe_unused]] int flags) noexcept
{
  /* Implementation:
   * - Search for all regions
   *   - For reach region
   *     if flags | MCL_CURRENT:
   *       go to dataspace and lock range
   *     if flags | MCL_FUTURE:
   *       go to rm and tell it to map all future pages with lock-flag
   *       (API extension here!)
   *     if flags | MCL_ONFAULT:
   *       see manpage...
   */

  return 0;
}

int munlockall(void) noexcept
{
  /* Implementation:
   * - Search for all regions
   *   - For each region
   *     go to dataspace and unlock range
   *     go to rm to remove unlock flag for mappings
   */
  return 0;
}

