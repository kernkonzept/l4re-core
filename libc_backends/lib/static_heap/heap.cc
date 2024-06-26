/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <bits/l4-malloc.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>

extern char __heap_start[];
extern char __heap_end[];
static char *heap_pos = __heap_start;

void *uclibc_morecore(long bytes)
{
  if (bytes <= 0)
    return heap_pos;

  if (__heap_end - heap_pos < bytes)
    return (void*)-1;

  void *ret = heap_pos;
  heap_pos += bytes;
  return ret;
}

extern "C" void *__libc_alloc_initial_tls(unsigned long size) __attribute__ ((__nothrow__));
void * __attribute__ ((__nothrow__)) __libc_alloc_initial_tls(unsigned long size)
{
  return uclibc_morecore(size);
}

void * __attribute__((weak))
mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
noexcept(noexcept(mmap(start, length, prot, flags, fd, offset)))
{
  errno = ENOMEM;
  return MAP_FAILED;
}

int __attribute__((weak))
munmap(void *start, size_t length) noexcept(noexcept(munmap(start, length)))
{
  errno = EINVAL;
  return -1;
}
