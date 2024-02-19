/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

pid_t fork(void)
{
  printf("Unimplemented: fork()\n");
  errno = -ENOMEM;
  return -1;
}

pid_t vfork(void)
{
  printf("Unimplemented: vfork()\n");
  errno = -ENOMEM;
  return -1;
}
