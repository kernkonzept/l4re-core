#define _GNU_SOURCE

#include "l4/sys/utcb.h"

#include <sys/mman.h>

#include <stdarg.h>
#include <string.h>

#define UTCB_SAVE                   \
  l4_msg_regs_t store;              \
  l4_msg_regs_t *mr = l4_utcb_mr(); \
  memcpy(&store, mr, sizeof(store));

#define UTCB_RESTORE memcpy(mr, &store, sizeof(store));

void *
__mmap_utcb_safe(void *start, size_t len, int prot, int flags, int fd,
                 off_t off)
{
  UTCB_SAVE;
  void *r = __mmap(start, len, prot, flags, fd, off);
  UTCB_RESTORE;
  return r;
}

void *
__mremap_utcb_safe(void *old_addr, size_t old_len, size_t new_len, int flags,
                   ...)
{
  UTCB_SAVE;
  void *r;
  if (flags & MREMAP_FIXED)
    {
      va_list ap;
      void *new_addr;

      va_start(ap, flags);
      new_addr = va_arg(ap, void *);
      va_end(ap);
      r = __mremap(old_addr, old_len, new_len, flags, new_addr);
    }
  else
    r = __mremap(old_addr, old_len, new_len, flags);
  UTCB_RESTORE;
  return r;
}

int
__munmap_utcb_safe(void *start, size_t len)
{
  UTCB_SAVE;
  int r = __munmap(start, len);
  UTCB_RESTORE;
  return r;
}
