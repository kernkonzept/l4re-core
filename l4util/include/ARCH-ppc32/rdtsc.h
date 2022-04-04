/**
 * \file
 * \brief  time stamp counter related functions
 *
 * \author  Frank Mehnert <fm3@os.inf.tu-dresden.de>
 * \ingroup l4util_tsc
 */

/*
 * (c) 2003-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef __l4_rdtsc_h
#define __l4_rdtsc_h

/**
 * \defgroup l4util_tsc Timestamp Counter
 * \ingroup l4util_api
 */

#include <l4/sys/compiler.h>
#include <l4/sys/l4int.h>
#include <l4/sys/kip.h>


EXTERN_C_BEGIN

extern l4_uint32_t l4_scaler_tsc_to_us;
extern l4_uint32_t l4_scaler_timer_to_tsc;

/* interface */
/**
 * \addtogroup l4util_tsc
 */
/**@{*/

/**
 * \brief Read current value of CPU-internal time stamp counter.
 * \return 64-bit time stamp
 */
L4_INLINE l4_cpu_time_t
l4_rdtsc (void);

/**
 * \brief Read the lest significant 32 bit of the TSC.
 *
 * Useful for smaller differences, needs less cycles.
 */
L4_INLINE
l4_uint32_t l4_rdtsc_32(void);

/** Convert time stamp into micro seconds value.
 * \param tsc time value in CPU ticks
 * \return time value in micro seconds
 */
L4_INLINE l4_uint64_t
l4_tsc_to_us (l4_cpu_time_t tsc);

/**
 * Calibrate scalers for time stamp calculations.
 *
 * Determine some scalers to be able to convert between real time and CPU
 * ticks. Just calls l4_tsc_init().
 */
L4_INLINE l4_uint32_t
l4_calibrate_tsc (l4_kernel_info_t *kip);

/**
 * \brief Initialitze scaler for TSC calicaltions.
 *
 * Initialize the scalers needed by l4_tsc_to_ns()/l4_ns_to_tsc() and so on.
 * Current versions of Fiasco export these scalers from kernel into userland.
 * The programmer may decide whether he allows to use these scalers or if an
 * calibration should be performed.
 * \param kip  KIP pointer
 * \return 0 on error (no scalers exported by kernel)
 *         otherwise returns (2^32 / (tsc per µsec)). This value has the
 *         same semantics as the value returned by the calibrate_delay_loop()
 *         function of the Linux kernel.
 */
L4_CV l4_uint32_t
l4_tsc_init (l4_kernel_info_t *kip);
/**@}*/

EXTERN_C_END

L4_INLINE l4_uint32_t
l4_calibrate_tsc (l4_kernel_info_t *kip)
{
  return l4_tsc_init(kip);
}

L4_INLINE l4_cpu_time_t
l4_rdtsc (void)
{
  l4_uint32_t upper = 0, lower = 0;
  l4_cpu_time_t tb;

  __asm__ __volatile__
    (" 1:                    \n"
     " mftbu %[upper]        \n"
     " mftb  %[lower]        \n"
     " mftbu %%r12           \n"
     " cmpw  %[upper], %%r12 \n"
     " bne- 1b               \n"
     : [upper] "=r" (upper),
       [lower] "=r" (lower)
     : "0"(upper),
       "1"(lower)
     : "r12");

  tb = upper;
  tb = (tb << 32 | lower);
  return tb * l4_scaler_timer_to_tsc;
}

L4_INLINE
l4_uint32_t l4_rdtsc_32(void) 
{
  l4_uint32_t lower;

  __asm__ __volatile__
    (" mftb %[lower]\n"
     : [lower] "=r" (lower));

 return lower * l4_scaler_timer_to_tsc;
}

L4_INLINE l4_uint64_t
l4_tsc_to_us (l4_cpu_time_t tsc)
{
  return tsc / l4_scaler_tsc_to_us;
}

#endif /* __l4_rdtsc_h */

