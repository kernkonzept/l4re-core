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

int
__munmap_utcb_safe(void *start, size_t len)
{
  UTCB_SAVE;
  int r = __munmap(start, len);
  UTCB_RESTORE;
  return r;
}
