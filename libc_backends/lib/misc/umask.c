/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <sys/stat.h>

mode_t umask(mode_t mask)
{
  printf("Unimplemented: %s(%d)\n", __func__, mask);
  return 0;
}
