#include <unistd.h>
#include <errno.h>

int sethostname(const char *name, size_t len)
{
  (void)name;
  (void)len;
  errno = EPERM;
  return -1;
}
