/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
//#include <sys/prctl.h>

int prctl(int option, unsigned long arg2, unsigned long arg3,
          unsigned long arg4, unsigned long arg5);
int prctl(int option, unsigned long arg2, unsigned long arg3,
          unsigned long arg4, unsigned long arg5)
{
  printf("prctl(%d, %lx, %lx, %lx, %lx): void\n",
         option, arg2, arg3, arg4, arg5);
  return 0;
}
