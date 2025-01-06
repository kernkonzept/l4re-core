/*
 * Copyright (C) 2018, 2023-2024 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <fcntl.h>
#include <stdio.h>

int posix_fadvise64(int fd, off64_t offset, off64_t len, int advice)
{
  printf("posix_fadvise64(%d, %lld, %lld, %d): void\n",
         fd, (unsigned long long)offset, (unsigned long long)len, advice);
  return 0;
}

int posix_fadvise(int fd, off_t offset, off_t len, int advice)
{
  printf("posix_fadvise(%d, %lld, %lld, %d): void\n",
         fd, (unsigned long long)offset, (unsigned long long)len, advice);
  return 0;
}
