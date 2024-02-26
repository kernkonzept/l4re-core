/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int lockf64(int fd, int cmd, __off64_t len)
{
  printf("Unimplemented: %s(%d, %d, %lld)\n", __func__, fd, cmd,
         (long long)len);
  errno = ENOSYS;
  return -1;
}
