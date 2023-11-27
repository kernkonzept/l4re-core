/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int daemon(int nochdir, int noclose)
{
  printf("Unimplemented: daemon(%d, %d)\n", nochdir, noclose);
  errno = -ENOMEM;
  return -1;
}
