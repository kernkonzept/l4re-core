/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdlib.h>
#include <stdio.h>

char *ptsname(int fd);
char *ptsname(int fd)
{
  printf("unimplemented: %s(%d)\n", __func__, fd);
  return "unimplemented-ptsname";
}
