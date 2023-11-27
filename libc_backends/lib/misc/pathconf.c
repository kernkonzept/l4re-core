/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

long fpathconf(int fd, int name)
{
  printf("Unimplemented: %s(%d, %d)\n", __func__, fd, name);
  errno = EINVAL;
  return -1;
}

long pathconf(const char *path, int name)
{
  printf("Unimplemented: %s(%s, %d)\n", __func__, path, name);
  errno = EINVAL;
  return -1;
}
