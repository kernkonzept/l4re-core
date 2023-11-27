/*
 * Copyright (C) 2009 TU Dresden.
 * Author(s): Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <stdlib.h>

int system(const char *path)
{
  (void)path;
  errno = EINVAL;
  return -1;
}
