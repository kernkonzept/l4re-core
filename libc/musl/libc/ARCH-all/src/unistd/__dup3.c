#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int
__dup3(int old, int new, int flags)
{
  if (old == new)
    {
      errno = EINVAL;
      return -1;
    }

  if (flags)
    {
      if (flags & ~O_CLOEXEC)
        {
          errno = EINVAL;
          return -1;
        }
    }

  int r;
  do
    {
      r = dup2(old, new);
    }
  while (r == -1 && errno == EBUSY);

  if (r >= 0 && (flags & O_CLOEXEC))
    fcntl(new, F_SETFD, FD_CLOEXEC);

  return r;
}
