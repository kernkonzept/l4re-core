#include "pthread-api.h"

#include <errno.h>

#ifdef L4_MINIMAL_LIBC

static int _errnoval;

int *__errno_location(void)
{
  return &_errnoval;
}

#else

int *__errno_location(void)
{
  return &__pthread_libc_data(__pthread_self())->errno_val;
}

#endif

weak_alias(__errno_location, ___errno_location);
