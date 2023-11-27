/*
 * Copyright (C) 2009, 2010, 2018 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/resource.h>

int getrlimit(__rlimit_resource_t resource, struct rlimit *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__, resource, rlim);
  errno = EINVAL;
  return -1;
}

int getrlimit64(__rlimit_resource_t resource, struct rlimit64 *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__, resource, rlim);
  errno = EINVAL;
  return -1;
}

int setrlimit(__rlimit_resource_t resource, const struct rlimit *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__,
         resource, rlim);
  errno = EINVAL;
  return -1;
}

#ifdef __USE_LARGEFILE64
int setrlimit64(__rlimit_resource_t resource, const struct rlimit64 *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__,
         resource, rlim);
  errno = EINVAL;
  return -1;
}
#else
#warning No large-file support enabled?
#endif
