/**
 * \file
 * \brief  time stamp counter related functions
 *
 * \author  Frank Mehnert <fm3@os.inf.tu-dresden.de>
 * \ingroup l4util_tsc
 */

/*
 * (c) 2003-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Jork Löser <jork@os.inf.tu-dresden.de>,
 *               Martin Pohlack <mp26@os.inf.tu-dresden.de>
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

/* interface */
/**
 * \addtogroup l4util_tsc
 */
/*@{*/

#define L4_TSC_INIT_AUTO             0 ///< Automatic init
#define L4_TSC_INIT_KERNEL           1 ///< Initialized by kernel
#define L4_TSC_INIT_CALIBRATE        2 ///< Initialized by user-level

extern l4_uint32_t l4_scaler_tsc_to_ns;
extern l4_uint32_t l4_scaler_tsc_to_us;
extern l4_uint32_t l4_scaler_ns_to_tsc;
extern l4_uint32_t l4_scaler_tsc_linux;

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

/**
 * \brief Return current value of CPU-internal performance measurement counter.
 * \param  nr		Number of counter (0 or 1)
 * \return 64-bit PMC */
L4_INLINE l4_cpu_time_t
l4_rdpmc (int nr);

/**
 * \brief Return the least significant 32 bit of a performance counter.
 *
 * Useful for smaller differences, needs less cycles.
 */
L4_INLINE
l4_uint32_t l4_rdpmc_32(int nr);

/** Convert time stamp to ns value.
 * \param tsc time value in CPU ticks
 * \return time value in ns
 */
L4_INLINE l4_uint64_t
l4_tsc_to_ns (l4_cpu_time_t tsc);

/** Convert time stamp into micro seconds value.
 * \param tsc time value in CPU ticks
 * \return time value in micro seconds
 */
L4_INLINE l4_uint64_t
l4_tsc_to_us (l4_cpu_time_t tsc);

/** Convert time stamp to s.ns value.
 * \param tsc time value in CPU ticks
 * \retval s seconds
 * \retval ns nano seconds
 */
L4_INLINE void
l4_tsc_to_s_and_ns (l4_cpu_time_t tsc, l4_uint32_t *s, l4_uint32_t *ns);

/**
 * \brief Convert nano seconds into CPU ticks.
 * \param ns nano seconds
 * \return CPU ticks
 */
L4_INLINE l4_cpu_time_t
l4_ns_to_tsc (l4_uint64_t ns);

/**
 * \brief Wait busy for a small amount of time.
 * \param ns nano seconds to wait
 * \attention Not intendet for any use!
 */
L4_INLINE void
l4_busy_wait_ns (l4_uint64_t ns);

/**
 * \brief Wait busy for a small amount of time.
 * \param us micro seconds to wait
 * \attention Not intendet for any use!
 */
L4_INLINE void
l4_busy_wait_us (l4_uint64_t us);

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

/**
 * \brief Get CPU frequency in Hz
 * \return frequency in Hz
 */
L4_CV l4_uint32_t
l4_get_hz (void);

/*@}*/

EXTERN_C_END

/* implementaion */

L4_INLINE l4_uint32_t
l4_calibrate_tsc (l4_kernel_info_t *kip)
{
  return l4_tsc_init(L4_TSC_INIT_AUTO, kip);
}

L4_INLINE l4_cpu_time_t
l4_rdtsc (void)
{
    l4_cpu_time_t v;

    __asm__ __volatile__
	("				\n\t"
	 ".byte 0x0f, 0x31		\n\t"
	/*"rdtsc\n\t"*/
	:
	"=A" (v)
	: /* no inputs */
	);

    return v;
}

/* the same, but only 32 bit. Useful for smaller differences,
   needs less cycles. */
L4_INLINE
l4_uint32_t l4_rdtsc_32(void)
{
  l4_uint32_t x;

  __asm__ __volatile__ (
       ".byte 0x0f, 0x31\n\t"	// rdtsc
       : "=a" (x)
       :
       : "edx");

  return x;
}

L4_INLINE l4_cpu_time_t
l4_rdpmc (int nr)
{
    l4_cpu_time_t v;

    __asm__ __volatile__ (
	 "rdpmc				\n\t"
	:
	"=A" (v)
	: "c" (nr)
	);

    return v;
}

/* the same, but only 32 bit. Useful for smaller differences,
   needs less cycles. */
L4_INLINE
l4_uint32_t l4_rdpmc_32(int nr)
{
  l4_uint32_t x;

  __asm__ __volatile__ (
       "rdpmc				\n\t"
       : "=a" (x)
       : "c" (nr)
       : "edx");

  return x;
}

L4_INLINE l4_uint64_t
l4_tsc_to_ns (l4_cpu_time_t tsc)
{
    l4_uint32_t dummy;
    l4_uint64_t ns;
    __asm__
	("				\n\t"
	 "movl  %%edx, %%ecx		\n\t"
	 "mull	%3			\n\t"
	 "movl	%%ecx, %%eax		\n\t"
	 "movl	%%edx, %%ecx		\n\t"
	 "mull	%3			\n\t"
	 "addl	%%ecx, %%eax		\n\t"
	 "adcl	$0, %%edx		\n\t"
	 "shld	$5, %%eax, %%edx	\n\t"
	 "shll	$5, %%eax		\n\t"
	:"=A" (ns),
	 "=&c" (dummy)
	:"0" (tsc),
	 "g" (l4_scaler_tsc_to_ns)
	);
    return ns;
}

L4_INLINE l4_uint64_t
l4_tsc_to_us (l4_cpu_time_t tsc)
{
    l4_uint32_t dummy;
    l4_uint64_t us;
    __asm__
	("				\n\t"
	 "movl  %%edx, %%ecx		\n\t"
	 "mull	%3			\n\t"
	 "movl	%%ecx, %%eax		\n\t"
	 "movl	%%edx, %%ecx		\n\t"
	 "mull	%3			\n\t"
	 "addl	%%ecx, %%eax		\n\t"
	 "adcl	$0, %%edx		\n\t"
	:"=A" (us),
	 "=&c" (dummy)
	:"0" (tsc),
	 "g" (l4_scaler_tsc_to_us)
	);
    return us;
}

L4_INLINE void
l4_tsc_to_s_and_ns (l4_cpu_time_t tsc, l4_uint32_t *s, l4_uint32_t *ns)
{
    l4_uint32_t dummy;
    __asm__
	("				\n\t"
	 "movl  %%edx, %%ecx		\n\t"
	 "mull	%4			\n\t"
	 "movl	%%ecx, %%eax		\n\t"
	 "movl	%%edx, %%ecx		\n\t"
	 "mull	%4			\n\t"
	 "addl	%%ecx, %%eax		\n\t"
	 "adcl	$0, %%edx		\n\t"
	 "movl  $1000000000, %%ecx	\n\t"
	 "shld	$5, %%eax, %%edx	\n\t"
	 "shll	$5, %%eax		\n\t"
	 "divl  %%ecx			\n\t"
	:"=a" (*s), "=d" (*ns), "=&c" (dummy)
	: "A" (tsc), "g" (l4_scaler_tsc_to_ns)
	);
}

L4_INLINE l4_cpu_time_t
l4_ns_to_tsc (l4_uint64_t ns)
{
    l4_uint32_t dummy;
    l4_cpu_time_t tsc;
    __asm__
	("				\n\t"
	 "movl  %%edx, %%ecx		\n\t"
	 "mull	%3			\n\t"
	 "movl	%%ecx, %%eax		\n\t"
	 "movl	%%edx, %%ecx		\n\t"
	 "mull	%3			\n\t"
	 "addl	%%ecx, %%eax		\n\t"
	 "adcl	$0, %%edx		\n\t"
	 "shld	$5, %%eax, %%edx	\n\t"
	 "shll	$5, %%eax		\n\t"
	:"=A" (tsc),
	 "=&c" (dummy)
	:"0" (ns),
	 "g" (l4_scaler_ns_to_tsc)
	);
    return tsc;
}

L4_INLINE void
l4_busy_wait_ns (l4_uint64_t ns)
{
  l4_cpu_time_t stop = l4_rdtsc();
  stop += l4_ns_to_tsc(ns);

  while (l4_rdtsc() < stop)
    ;
}

L4_INLINE void
l4_busy_wait_us (l4_uint64_t us)
{
  l4_cpu_time_t stop = l4_rdtsc ();
  stop += l4_ns_to_tsc(us*1000ULL);

  while (l4_rdtsc() < stop)
    ;
}

EXTERN_C_END

#endif /* __l4_rdtsc_h */

