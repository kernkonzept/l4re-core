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
 * Return 2^32 / (tsc clocks per usec)
 *
 * Note, l4_tsc_init(L4_TSC_INIT_CALIBRATE) needs to have
 * I/O ports 0x20, 0x42, 0x43 and 0x61
 */
L4_CV l4_uint32_t
l4_tsc_init (int constraint, l4_kernel_info_t *kip)
{
  l4_scaler_tsc_linux = 0;

  if (constraint != L4_TSC_INIT_CALIBRATE)
    {
      /*
       * First, lets try to get the info out of the kernel info page so that
       * we don't need to do port i/o. If we're unable to get the information
       * there, measure it ourselves.
       */
      if (kip)
	{
	  if (kip->frequency_cpu
	      && kip->frequency_cpu < 50000000 /* sanity check*/)
	    {
	      l4_scaler_tsc_linux = muldiv(1U<<30, 4000, kip->frequency_cpu);
	      l4_scaler_ns_to_tsc = muldiv(1U<<27, kip->frequency_cpu, 1000000);

	      /* l4_scaler_ns_to_tsc = (2^32 * Hz) / (32 * 1.000.000.000) */
	    }
	  else
	    printf("CPU frequency not set in KIP or invalid.\n");
	}
      else
	printf("No KIP available!\n");
    }
  if (   (l4_scaler_tsc_linux == 0)
      && (constraint != L4_TSC_INIT_KERNEL))
    {
      const unsigned clock_tick_rate = 1193180;
      const unsigned calibrate_time  = 50000 /*us*/ + 1;
      const unsigned calibrate_latch = clock_tick_rate / 20; /* 20Hz = 50ms */

  //    l4_umword_t flags;
      l4_uint64_t tsc_start, tsc_end;
      register l4_uint32_t count;

      /* disable interrupts */
  //    l4util_flags_save(&flags);
  //    l4util_cli();

      /* Set the Gate high, disable speaker */
      l4util_out8 ((l4util_in8 (0x61) & ~0x02) | 0x01, 0x61);

      l4util_out8 (0xb0, 0x43);		/* binary, mode 0, LSB/MSB, Ch 2 */
      l4util_out8 (calibrate_latch & 0xff, 0x42);	/* LSB of count */
      l4util_out8 (calibrate_latch   >> 8, 0x42);	/* MSB of count */

      tsc_start = l4_rdtsc ();
      count = 0;
      do
	{
	  count++;
	}
      while ((l4util_in8 (0x61) & 0x20) == 0);
      tsc_end = l4_rdtsc ();

      /* restore flags */
  //    l4util_flags_restore(&flags);

      /* Error: ECTCNEVERSET */
      if (count <= 1)
	goto bad_ctc;

      /* 64-bit subtract - gcc just messes up with long longs */
      tsc_end -= tsc_start;

      /* Error: ECPUTOOFAST */
      if (tsc_end & 0xffffffff00000000LL)
	goto bad_ctc;

      /* Error: ECPUTOOSLOW */
      if ((tsc_end & 0xffffffffL) <= calibrate_time)
	goto bad_ctc;

      l4_scaler_tsc_linux = muldiv(1U<<30, (1U<<2) * calibrate_time, (l4_uint32_t)tsc_end);
      l4_scaler_ns_to_tsc = muldiv(((1ULL<<32)/1000ULL), (l4_uint32_t)tsc_end,
                                   calibrate_time * (1<<5));
    }

  l4_scaler_tsc_to_ns = muldiv(l4_scaler_tsc_linux, 1000, 1<<5);
  l4_scaler_tsc_to_us =        l4_scaler_tsc_linux;

  return l4_scaler_tsc_linux;

  /*
   * The CTC wasn't reliable: we got a hit on the very first read,
   * or the CPU was so fast/slow that the quotient wouldn't fit in
   * 32 bits..
   */
bad_ctc:
  return 0;
}

L4_CV l4_uint32_t
l4_get_hz (void)
{
  if (!l4_scaler_tsc_to_ns)
    return 0;

  return (l4_uint32_t)(l4_ns_to_tsc(1000000000UL));
}

