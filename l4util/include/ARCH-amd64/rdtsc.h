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
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
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
/**@{*/

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
 * \param  ecx          ECX value for the rdpmc instruction. For details see
 *                      the Intel IA-32 Architectures Software Developer's
 *                      Manual.
 * \return 64-bit PMC */
L4_INLINE l4_uint64_t
l4_rdpmc (int ecx);

/**
 * \brief Return the least significant 32 bit of a performance counter.
 *
 * Useful for smaller differences, needs less cycles.
 */
L4_INLINE
l4_uint32_t l4_rdpmc_32(int ecx);

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
 * \attention Not intended for any use!
 */
L4_INLINE void
l4_busy_wait_ns (l4_uint64_t ns);

/**
 * \brief Wait busy for a small amount of time.
 * \param us micro seconds to wait
 * \attention Not intended for any use!
 */
L4_INLINE void
l4_busy_wait_us (l4_uint64_t us);

/**
 * Determine scalers for time stamp calculations.
 *
 * Determine some scalers to be able to convert between real time and CPU
 * ticks. Just calls l4_tsc_init().
 */
L4_INLINE l4_uint32_t
l4_calibrate_tsc (l4_kernel_info_t *kip);

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
l4_tsc_init (l4_kernel_info_t *kip);

/**
 * \brief Get CPU frequency in Hz
 * \return frequency in Hz
 */
L4_CV l4_uint32_t
l4_get_hz (void);

/**@}*/

EXTERN_C_END

/* implementation */

L4_INLINE l4_uint32_t
l4_calibrate_tsc (l4_kernel_info_t *kip)
{
  return l4_tsc_init(kip);
}

L4_INLINE l4_cpu_time_t
l4_rdtsc (void)
{
  l4_umword_t lo, hi;

  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));

  return ((l4_cpu_time_t)hi << 32) | lo;
}

L4_INLINE l4_uint64_t
l4_rdpmc (int ecx)
{
  l4_umword_t lo, hi;

  __asm__ __volatile__ ("rdpmc" : "=a"(lo), "=d"(hi) : "c"(ecx));

  return ((l4_cpu_time_t)hi << 32) | lo;
}

L4_INLINE
l4_uint32_t l4_rdtsc_32(void)
{
  l4_umword_t lo, hi;

  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));

  return lo;
}

L4_INLINE
l4_uint32_t l4_rdpmc_32(int ecx)
{
  l4_umword_t lo, hi;

  __asm__ __volatile__ ("rdpmc" : "=a"(lo), "=d"(hi) : "c"(ecx));

  return lo;
}

L4_INLINE l4_uint64_t
l4_tsc_to_ns (l4_cpu_time_t tsc)
{
    l4_uint64_t ns, dummy;
    __asm__
	("				\n\t"
	 "mulq	%3			\n\t"
         "shrd  $27, %%rdx, %%rax       \n\t"
	:"=a" (ns), "=d"(dummy)
	:"a" (tsc), "r" ((l4_uint64_t)l4_scaler_tsc_to_ns)
	);
    return ns;
}

L4_INLINE l4_uint64_t
l4_tsc_to_us (l4_cpu_time_t tsc)
{
    l4_uint64_t ns, dummy;
    __asm__
	("				\n\t"
	 "mulq	%3			\n\t"
         "shrd  $32, %%rdx, %%rax       \n\t"
	:"=a" (ns), "=d" (dummy)
	:"a" (tsc), "r" ((l4_uint64_t)l4_scaler_tsc_to_us)
	);
    return ns;
}

L4_INLINE void
l4_tsc_to_s_and_ns (l4_cpu_time_t tsc, l4_uint32_t *s, l4_uint32_t *ns)
{
    __asm__
	("				\n\t"
	 "mulq	%3			\n\t"
         "shrd  $27, %%rdx, %%rax       \n\t"
         "xorq  %%rdx, %%rdx            \n\t"
	 "divq  %4			\n\t"
	:"=a" (*s), "=&d" (*ns)
	: "a" (tsc), "r" ((l4_uint64_t)l4_scaler_tsc_to_ns),
          "rm"(1000000000ULL)
	);
}

L4_INLINE l4_cpu_time_t
l4_ns_to_tsc (l4_uint64_t ns)
{
    l4_uint64_t tsc, dummy;
    __asm__
        ("                              \n\t"
	 "mulq	%3			\n\t"
         "shrd  $27, %%rdx, %%rax       \n\t"
	:"=a" (tsc), "=d" (dummy)
	:"a" (ns), "r" ((l4_uint64_t)l4_scaler_ns_to_tsc)
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

