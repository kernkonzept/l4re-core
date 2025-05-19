#include <errno.h>

static int _errnoval;

int *__errno_location(void)
{
  return &_errnoval;
}

weak_alias(__errno_location, ___errno_location);
