/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#define _GNU_SOURCE 1  // needed for mremap
#include <sys/mman.h>
#include <bits/l4-malloc.h>
#include <errno.h>


#include <l4/bid_config.h>
#include <l4/re/mem_alloc>
#include <l4/re/dataspace>
#include <l4/re/error_helper>
#include <l4/cxx/iostream>

#include "globals.h"

enum {
  Heap_max = L4_PAGESIZE * 64,
};

extern char __executable_start[];
static char *morecore_pos;
static char *morecore_end;

static void *mc_err_msg(long bytes, char const *msg)
{
  L4::cout << "l4re_kernel: ERROR: more-mem(" << L4::hex << bytes << "): "
           << msg << ".\n";
  errno = ENOMEM;
  return reinterpret_cast<void *>(-1UL);
}

void *uclibc_morecore(long bytes)
{
  using L4Re::Mem_alloc;
  using L4Re::Dataspace;

  // A positive value allocates memory. Make sure we have some...
  if (bytes > 0 && !morecore_end)
    {
      // first call allocates a dataspace
      L4::Cap<L4Re::Dataspace> heap;
      heap = Global::cap_alloc->alloc<L4Re::Dataspace>();
      if (!heap)
        return mc_err_msg(bytes, "Failed to allocate cap");
      if (Global::allocator->alloc(Heap_max, heap) < 0)
        return mc_err_msg(bytes, "Failed to allocate memory");

      L4Re::Rm::Flags rm_flags(L4Re::Rm::F::RW);
#if defined(CONFIG_MMU)
      char *hp = __executable_start + 0x100000;
#else
      char *hp = 0;
      rm_flags |= L4Re::Rm::F::Search_addr;
#endif
      if (L4Re::Env::env()->rm()->attach(&hp, Heap_max, rm_flags,
                                         L4::Ipc::make_cap_rw(heap), 0) < 0)
        {
          Global::cap_alloc->free(heap);
          return mc_err_msg(bytes, "Failed to attach memory");
        }

      morecore_pos = hp;
      morecore_end = hp + Heap_max;
    }

  // Allocate memory and return old position. With a negative value, 'free'
  // wants to return memory. Calling morecore with 0 size is done by the
  // malloc/free implementation to check for the amount of memory it got from
  // the last call to morecore.
  if (morecore_end - morecore_pos >= bytes)
    {
      char *prev_pos = morecore_pos;
      morecore_pos += bytes;
      return prev_pos;
    }

  return mc_err_msg(bytes, "Cannot provide more memory");
}

void * mmap(void *start, size_t length, int prot,
            int flags, int fd, off_t offset)
noexcept(noexcept(mmap(start, length, prot, flags, fd, offset)))
{
  L4::cout << "l4re_kernel: ERROR: mmap() called: unimplemented! size=" << length << "\n";
  errno = ENOMEM;
  return MAP_FAILED;
}

int munmap(void *start, size_t length) noexcept(noexcept(munmap(start, length)))
{
  L4::cout << "l4re_kernel: ERROR: munmap() called: unimplemented!\n";
  errno = EINVAL;
  return -1;
}

void *mremap(void *old_address, size_t old_size, size_t new_size,
             int may_move, ...)
noexcept(noexcept(mremap(old_address, old_size, new_size, may_move)))
{
  L4::cout << "l4re_kernel: ERROR: mremap() called: unimplemented!\n";
  errno = EINVAL;
  return MAP_FAILED;
}
