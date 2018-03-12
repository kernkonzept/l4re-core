/**
 * \file
 * \brief  time stamp counter related functions
 *
 * \date   Frank Mehnert <fm3@os.inf.tu-dresden.de>
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
/*@{*/

#define L4_TSC_INIT_AUTO             0 ///< Automatic init
#define L4_TSC_INIT_KERNEL           1 ///< Initialized by kernel
#define L4_TSC_INIT_CALIBRATE        2 ///< Initialized by user-level

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

EXTERN_C_BEGIN

/**
 * \brief Calibrate scalers for time stamp calculations.
 *
 * Determine some scalers to be able to convert between real time and CPU
 * ticks. This test uses channel 0 of the PIT (i8254) or the kernel KIP,
 * depending on availability.
 * Just calls l4_tsc_init(L4_TSC_INIT_AUTO).
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
 * \param   constraint   programmers constraint:
 *                       - #L4_TSC_INIT_AUTO if the kernel exports the scalers
 *                         then use them. If not, perform calibration using
 *                         channel 0 of the PIT (i8254). The latter case may
 *                         lead into short (unpredictable) periods where
 *                         interrupts are disabled.
 *                       - #L4_TSC_INIT_KERNEL depend on retrieving the scalers
 *                         from kernel. If the scalers are not available,
 *                         return 0.
 *                       - #L4_TSC_INIT_CALIBRATE Ignore possible scalers
 *                         exported by the scaler, instead insist on
 *                         calibration using the PIT.
 * \param kip            KIP pointer
 * \return 0 on error (no scalers exported by kernel, calibrating failed ...)
 *         otherwise returns (2^32 / (tsc per µsec)). This value has the
 *         same semantics as the value returned by the calibrate_delay_loop()
 *         function of the Linux kernel.
 */
L4_CV l4_uint32_t
l4_tsc_init (int constraint, l4_kernel_info_t *kip);
/*@}*/

EXTERN_C_END

EXTERN_C_BEGIN

L4_INLINE l4_uint32_t
l4_calibrate_tsc (l4_kernel_info_t *kip)
{
  return l4_tsc_init(0, kip);
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

EXTERN_C_END
#endif /* __l4_rdtsc_h */

