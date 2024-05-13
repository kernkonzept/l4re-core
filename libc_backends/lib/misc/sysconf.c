/*
 * Copyright (C) 2008 TU Dresden,
 *               2014 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *            Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <sched.h>
#include <unistd.h>

int __sched_cpucount(size_t __setsize, const cpu_set_t *__setp)
{
  (void)__setsize;
  (void)__setp;
  return 4; // just some number
}

long sysconf(int name)
{
  switch (name)
  {
  case _SC_NPROCESSORS_ONLN:
    return __sched_cpucount(0, NULL);
  case _SC_PAGE_SIZE:
    return L4_PAGESIZE;
  case _SC_CLK_TCK:
    return 1000;
  case _SC_MONOTONIC_CLOCK:
    return 200112L;
  case _SC_OPEN_MAX:
    return 512;
  case _SC_CHILD_MAX:
    return 2000;
  default:
    break;
  }
  printf("%s: unknown command, name=%d\n", __func__, name);
  return 0;
}
