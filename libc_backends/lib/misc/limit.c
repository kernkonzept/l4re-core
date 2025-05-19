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

#ifdef CONFIG_L4_LIBC_UCLIBC
typedef __rlimit_resource_t res_t;
#else
typedef int res_t;
#endif

int getrlimit(res_t resource, struct rlimit *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__, resource, rlim);
  errno = EINVAL;
  return -1;
}

#ifndef CONFIG_L4_LIBC_MUSL
int getrlimit64(res_t resource, struct rlimit64 *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__, resource, rlim);
  errno = EINVAL;
  return -1;
}
#endif

int setrlimit(res_t resource, const struct rlimit *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__,
         resource, rlim);
  errno = EINVAL;
  return -1;
}

#if defined(__USE_LARGEFILE64) && !defined(CONFIG_L4_LIBC_MUSL)
int setrlimit64(res_t resource, const struct rlimit64 *rlim)
{
  printf("Unimplemented: %s(%d, %p)\n", __func__,
         resource, rlim);
  errno = EINVAL;
  return -1;
}
#endif
