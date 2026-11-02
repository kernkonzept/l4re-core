#include "pthread-api.h"

#include <netdb.h>

#ifdef L4_MINIMAL_LIBC

static int _h_errnoval;

int *__h_errno_location(void)
{
  return &_h_errnoval;
}

#else

int *__h_errno_location(void)
{
  return &__pthread_libc_data(__pthread_self())->h_errno_val;
}

#endif
