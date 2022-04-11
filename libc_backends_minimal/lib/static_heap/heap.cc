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

  if (heap_pos + bytes > __heap_end)
    return (void*)-1;

  void *ret = heap_pos;
  heap_pos += bytes;
  return ret;
}

void *mmap(void * /*addr*/, size_t /*length*/, int /*prot*/, int /*flags*/,
           int /*fd*/, off_t /*offset*/) __THROW
{
  errno = ENOSYS;
  return MAP_FAILED;
}

int munmap(void * /*addr*/, size_t /*length*/) __THROW
{
  errno = ENOSYS;
  return -1;
}

void *mremap (void * /*addr*/, size_t /*old_len*/, size_t /*new_len*/,
		     int /*flags*/, ...) __THROW
{
  errno = ENOSYS;
  return MAP_FAILED;
}

extern "C" void *__libc_alloc_initial_tls(unsigned long size) __attribute__ ((__nothrow__));
void *__libc_alloc_initial_tls(unsigned long size)
{
  return uclibc_morecore(size);
}
