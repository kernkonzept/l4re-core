/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/sys/types.h>
#include <l4/util/rdtsc.h>

l4_uint32_t l4_scaler_tsc_to_us;
l4_uint32_t l4_scaler_timer_to_tsc;

l4_uint32_t
l4_tsc_init (int constraint, l4_kernel_info_t *kip)
{
  (void)constraint;
  l4_scaler_timer_to_tsc = (kip->frequency_cpu * 1000)
                           / (kip->frequency_bus / 4);
  l4_scaler_tsc_to_us = (kip->frequency_cpu) / 1000;

  return 1;
}


