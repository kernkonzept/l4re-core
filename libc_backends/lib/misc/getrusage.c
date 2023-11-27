/*
 * Copyright (C) 2013 TU Dresden.
 * Author(s): Björn DÖbel <doebel@os.inf.tu-dresden.de>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <sys/resource.h>

int getrusage(__rusage_who_t who, struct rusage* usage)
{
  (void)who; (void)usage;
  errno = EINVAL;
  return -1;
}
