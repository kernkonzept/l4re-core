/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>

int kill(pid_t pid, int sig)
{
  printf("Unimplemented: kill(%d, %d)\n", pid, sig);
  errno = -EINVAL;
  return -1;
}
