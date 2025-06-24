/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/sys/types.h>
#include <l4/util/rdtsc.h>

l4_uint32_t l4_scaler_tsc_to_us;
l4_uint32_t l4_scaler_timer_to_tsc;

l4_uint32_t
l4_tsc_init(l4_kernel_info_t const *kip)
{
  l4_scaler_timer_to_tsc = kip->frequency_cpu * 1000;
  l4_scaler_tsc_to_us = (kip->frequency_cpu) / 1000;

  return 1;
}


