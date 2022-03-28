/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/sys/types.h>
#include <l4/sys/ipc.h>
#include <l4/sys/kip.h>
#include <l4/util/port_io.h>
#include <l4/util/rdtsc.h>
#include <stdio.h>

l4_uint32_t l4_scaler_tsc_to_ns;
l4_uint32_t l4_scaler_tsc_to_us;
l4_uint32_t l4_scaler_ns_to_tsc;
l4_uint32_t l4_scaler_tsc_linux;


static inline l4_uint32_t
muldiv (l4_uint32_t a, l4_uint32_t mul, l4_uint32_t div)
{
  l4_uint32_t dummy;
  asm ("mull %4 ; divl %3\n\t"
       :"=a" (a), "=d" (dummy)
       :"a" (a), "c" (div), "d" (mul));
  return a;
}


/*
 * Return 2^32 / (TSC clocks per usec).
 */
L4_CV l4_uint32_t
l4_tsc_init(l4_kernel_info_t *kip)
{
  if (!l4_scaler_tsc_linux)
    {
      if (!kip)
        {
          printf("No KIP available!\n");
          return 0;
        }

      if (!kip->frequency_cpu || kip->frequency_cpu >= 50000000)
        {
          printf("CPU frequency not set in KIP or invalid.\n");
          return 0;
        }

      /* tsc_linux = (2^30 * 4000) / (Hz / 1000) = (2^32 * 1000000) / Hz */
      l4_scaler_tsc_linux = muldiv(1U<<30, 4000, kip->frequency_cpu);
      /* ns_to_tsc = (2^32 * Hz) / (32 * 1000000000) */
      l4_scaler_ns_to_tsc = muldiv(1U<<27, kip->frequency_cpu, 1000000);
      /* tsc_to_ns = (((2^32 * 1000000) / Hz) * 1000) / 32
       *           = (2^27 * 1000000000) / Hz */
      l4_scaler_tsc_to_ns = muldiv(l4_scaler_tsc_linux, 1000, 1<<5);
      /* tsc_to_us = (2^32 * 1000000) / Hz */
      l4_scaler_tsc_to_us = l4_scaler_tsc_linux;
    }

  return l4_scaler_tsc_linux;
}

L4_CV l4_uint32_t
l4_get_hz (void)
{
  if (!l4_scaler_tsc_to_ns)
    return 0;

  return (l4_uint32_t)(l4_ns_to_tsc(1000000000UL));
}

