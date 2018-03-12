/*
 * Copyright (C) 2014 Imagination Technologies Ltd.
 * Author: Yann Le Du <ledu@kymasys.com>
 */

#include <l4/util/backtrace.h>

int
l4util_backtrace(void **pc_array, int max)
{
  (void)pc_array;
  (void)max;
  return 0;
}
