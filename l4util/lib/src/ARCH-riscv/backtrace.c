/* SPDX-License-Identifier: LGPL-2.1-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */
#include <l4/util/backtrace.h>

int
l4util_backtrace(void **pc_array, int max)
{
  (void)pc_array;
  (void)max;
  return 0;
}
