/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

pid_t waitpid(pid_t pid, int *status, int options)
{
  printf("Unimplemented: %s(%d)\n", __func__, pid);
  (void)status;
  (void)options;
  errno = EINVAL;
  return -1;
}
