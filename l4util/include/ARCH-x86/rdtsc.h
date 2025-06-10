/**
 * \file
 * Timestamp counter related functions
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
 * License: see LICENSE.spdx (in this directory or the directories above)
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

L4_BEGIN_DECLS

/* interface */
/**
 * \addtogroup l4util_tsc
 */
/**@{*/

extern l4_uint32_t l4_scaler_tsc_to_ns;
extern l4_uint32_t l4_scaler_tsc_to_us;
extern l4_uint32_t l4_scaler_ns_to_tsc;
extern l4_uint32_t l4_scaler_tsc_linux;

/**
 * Read current value of CPU-internal timestamp counter.
 * \return 64-bit timestamp
 */
L4_INLINE l4_cpu_time_t
l4_rdtsc (void);

/**
 * Read the lest significant 32 bit of the TSC.
 *
 * Useful for smaller differences, needs less cycles.
 */
L4_INLINE
l4_uint32_t l4_rdtsc_32(void);

/**
 * Return current value of CPU-internal performance measurement counter.
 * \param  ecx          ECX value for the rdpmc instruction. For details see
 *                      the Intel IA-32 Architectures Software Developer's
 *                      Manual.
 * \return 64-bit PMC */
L4_INLINE l4_uint64_t
l4_rdpmc (int ecx);

/**
 * Return the least significant 32 bit of a performance counter.
 *
 * Useful for smaller differences, needs less cycles.
 */
L4_INLINE
l4_uint32_t l4_rdpmc_32(int ecx);

/** Convert timestamp to ns value.
 * \param tsc time value in CPU ticks
 * \return time value in ns
 */
L4_INLINE l4_uint64_t
l4_tsc_to_ns (l4_cpu_time_t tsc);

/** Convert timestamp into micro seconds value.
 * \param tsc time value in CPU ticks
 * \return time value in micro seconds
 */
L4_INLINE l4_uint64_t
l4_tsc_to_us (l4_cpu_time_t tsc);

/** Convert timestamp to s.ns value.
 * \param      tsc  time value in CPU ticks
 * \param[out] s    seconds
 * \param[out] ns   nano seconds
 */
L4_INLINE void
l4_tsc_to_s_and_ns (l4_cpu_time_t tsc, l4_uint32_t *s, l4_uint32_t *ns);

/**
 * Convert nano seconds into CPU ticks.
 * \param ns nano seconds
 * \return CPU ticks
 */
L4_INLINE l4_cpu_time_t
l4_ns_to_tsc (l4_uint64_t ns);

/**
 * Wait busy for a small amount of time.
 * \param ns nano seconds to wait
 * \attention Not intended for any use!
 */
L4_INLINE void
l4_busy_wait_ns (l4_uint64_t ns);

/**
 * Wait busy for a small amount of time.
 * \param us micro seconds to wait
 * \attention Not intended for any use!
 */
L4_INLINE void
l4_busy_wait_us (l4_uint64_t us);

/**
 * Determine scalers for timestamp calculations.
 *
 * Determine some scalers to be able to convert between real time and CPU
 * ticks. Just calls l4_tsc_init().
 */
L4_INLINE l4_uint32_t
l4_calibrate_tsc(l4_kernel_info_t const *kip);

/**
 * Initialize scaler for TSC calibrations from the kernel.
 *
 * Initialize the scalers needed by l4_tsc_to_ns()/l4_ns_to_tsc() and so on.
 * Use the kernel-provided frequency.
 *
 * \param kip  KIP pointer
 *
 * \return 0 on error (no scalers exported by kernel)
 *         otherwise returns (2^32 / (tsc per µsec)). This value has the
 *         same semantics as the value returned by the calibrate_delay_loop()
 *         function of the Linux kernel.
 */
L4_CV l4_uint32_t
l4_tsc_init(l4_kernel_info_t const *kip);

/**
 * Get CPU frequency in Hz
 * \return frequency in Hz
 */
L4_CV l4_uint32_t
l4_get_hz (void);

/**@}*/

L4_END_DECLS

/* implementation */

L4_INLINE l4_uint32_t
l4_calibrate_tsc(l4_kernel_info_t const *kip)
{
  return l4_tsc_init(kip);
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

L4_INLINE l4_uint64_t
l4_rdpmc (int ecx)
{
    l4_cpu_time_t v;

    __asm__ __volatile__ (
	 "rdpmc				\n\t"
	:
	"=A" (v)
	: "c" (ecx)
	);

    return v;
}

/* the same, but only 32 bit. Useful for smaller differences,
   needs less cycles. */
L4_INLINE
l4_uint32_t l4_rdpmc_32(int ecx)
{
  l4_uint32_t x;

  __asm__ __volatile__ (
       "rdpmc				\n\t"
       : "=a" (x)
       : "c" (ecx)
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

#endif /* __l4_rdtsc_h */

