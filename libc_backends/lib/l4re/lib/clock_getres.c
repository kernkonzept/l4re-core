/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <time.h>

int clock_getres(clockid_t clock_id, struct timespec * res)
{
  if (clock_id != CLOCK_REALTIME && clock_id != CLOCK_MONOTONIC)
    {
      errno = EINVAL;
      return -1;
    }

  if (res)
    {
      res->tv_sec = 0;
      res->tv_nsec = 1000;
    }

  return 0;
}
