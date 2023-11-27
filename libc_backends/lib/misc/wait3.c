/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

pid_t wait3(int *status, int options, struct rusage *rusage)
{
  printf("Unimplemented: %s(%p, %d, %p)\n", __func__,
         status, options, rusage);
  errno = EPERM;
  return -1;
}
