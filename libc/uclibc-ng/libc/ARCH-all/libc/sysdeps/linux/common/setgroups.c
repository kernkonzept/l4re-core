
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <grp.h>

int setgroups(size_t size, const gid_t *list)
{
  (void)size;
  (void)list;
  printf("Unimplemented: %s\n", __func__);
  errno = EPERM;
  return -1;
}
libc_hidden_def(setgroups)
