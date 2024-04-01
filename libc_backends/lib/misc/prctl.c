/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Adam Lackorzynski <adam@l4re.org>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <sys/prctl.h>

int prctl(int __option, ...)
{
  printf("prctl(%d, ...): void\n", __option);
  return 0;
}
