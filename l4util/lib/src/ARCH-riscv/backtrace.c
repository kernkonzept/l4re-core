/*
 * Copyright (C) 2021, 2024 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/util/backtrace.h>

int
l4util_backtrace(void **pc_array, int max)
{
  (void)pc_array;
  (void)max;
  return 0;
}
