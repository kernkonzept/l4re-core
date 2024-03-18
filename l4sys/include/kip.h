/**
 * \file
 * Kernel Info Page access functions.
 * \ingroup l4_api
 */
/*
 * (c) 2008-2013 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#include <l4/sys/compiler.h>
#include <l4/sys/l4int.h>

#include <l4/sys/__kip-arch.h>

/**
 * \internal
 */
struct l4_kip_platform_info
{
  char                             name[16];
  l4_uint32_t                      is_mp;
  struct l4_kip_platform_info_arch arch;
};

#if L4_MWORD_BITS == 32
#  include <l4/sys/__kip-32bit.h>
#else
#  include <l4/sys/__kip-64bit.h>
#endif

/**
 * \addtogroup l4_kip_api
 *
 * C interface for the Kernel Interface Page:<br>
 * \includefile{l4/sys/kip.h}
 */
/**@{*/

/**
 * \internal
 */
enum l4_kernel_info_consts_t
{
  L4_KIP_VERSION_FIASCO      = 0x87004444,
  L4_KIP_VERSION_FIASCO_MASK = 0xff00ffff,
};

enum
{
  /**
   * Offset of KIP code (provided by the kernel) for reading the KIP clock in
   * microseconds. If the kernel is configured for a fine-grained KIP clock
   * (CONFIG_SYNC_TSC enabled for IA32, ARM_SYNC_CLOCK for ARM), this code
   * provides the KIP clock with microseconds granularity and accuracy by
   * reading the hardware clock used by the kernel and transforming this value
   * into microseconds. Otherwise this code just reads the KIP clock value.
   */
  L4_KIP_OFFS_READ_US        = 0x900,

  /**
   * Offset of KIP code (provided by the kernel) for reading the time stamp
   * counter and transforming this value into nanoseconds. If the kernel is
   * configured for fine-grained KIP clock (CONFIG_SYNC enabled for IA32,
   * ARM_SYNC_CLOCK for ARM), this code provides the KIP clock with nanoseconds
   * granularity and accuracy by reading the hardware clock used by the kernel
   * and transforming this value into nanoseconds. Otherwise this code just
   * reads the KIP clock value and multiplies it by 1000.
   */
  L4_KIP_OFFS_READ_NS        = 0x980,
};

/**
 * \internal
 */
extern l4_kernel_info_t const *l4_global_kip;

/**
 * Kernel Info Page identifier ("L4µK").
 */
#define L4_KERNEL_INFO_MAGIC (0x4BE6344CL) /* "L4µK" */


/**
 * Get Kernel Info Page.
 *
 * \return Pointer to Kernel Info Page (KIP) structure.
 */
L4_INLINE l4_kernel_info_t const *l4_kip(void) L4_NOTHROW;


/**
 *  Get the kernel version.
 *
 * \param kip  Kernel Info Page.
 *
 * \return Kernel version string. 0 if KIP could not be mapped.
 */
L4_INLINE l4_umword_t l4_kip_version(l4_kernel_info_t const *kip) L4_NOTHROW;

/**
 *  Get the kernel version string.
 *
 * \param kip  Kernel Info Page.
 *
 * \return Kernel version string.
 */
L4_INLINE const char *l4_kip_version_string(l4_kernel_info_t const *kip) L4_NOTHROW;

/**
 * Return offset in bytes of version_strings relative to the KIP base.
 *
 * \param kip  Pointer to the kernel info page (KIP).
 *
 * \return offset of version_strings relative to the KIP base address, in
 *         bytes.
 */
L4_INLINE int
l4_kernel_info_version_offset(l4_kernel_info_t const *kip) L4_NOTHROW;

/**
 * Return clock value from the KIP.
 *
 * \param kip  Pointer to the kernel info page (KIP).
 *
 * \return Value of the clock field in the KIP.
 *
 * The KIP clock always contains the current (relative) time in micro seconds
 * independently of the CPU frequency. The clock is only guaranteed to be
 * accurate within the scheduling granularity announced in the KIP.
 *
 * This function basically calls the KIP code for reading the KIP clock with
 * microseconds resolution. The accuracy depends on the platform and the kernel
 * configuration.
 *
 * \see L4_KIP_OFFS_READ_US.
 */
L4_INLINE l4_cpu_time_t
l4_kip_clock(l4_kernel_info_t const *kip) L4_NOTHROW;

/**
 * Return least significant machine word of clock value from the KIP.
 *
 * \param kip  Pointer to the kernel info page (KIP).
 *
 * \return Lower machine word of clock value from the KIP.
 *
 * This function will always provide the least significant machine word of the
 * clock value from the KIP, regardless of the kernel configuration.
 */
L4_INLINE l4_umword_t
l4_kip_clock_lw(l4_kernel_info_t const *kip) L4_NOTHROW;

/**
 * Return current clock using the KIP in nanoseconds.
 *
 * \param kip  Pointer to the kernel info page (KIP).
 *
 * \return Value of the current clock in nanoseconds.
 *
 * This function basically calls the KIP code for reading the KIP clock with
 * nanoseconds resolution. The accuracy depends on the platform and the kernel
 * configuration.
 *
 * \see L4_KIP_OFFS_READ_NS.
 */
L4_INLINE l4_uint64_t
l4_kip_clock_ns(l4_kernel_info_t const *kip) L4_NOTHROW;

/**@}*/

/*************************************************************************
 * Implementations
 *************************************************************************/

L4_INLINE l4_kernel_info_t const*
l4_kip(void) L4_NOTHROW
{ return l4_global_kip; }

L4_INLINE l4_umword_t
l4_kip_version(l4_kernel_info_t const *kip) L4_NOTHROW
{ return kip->version & L4_KIP_VERSION_FIASCO_MASK; }

L4_INLINE const char*
l4_kip_version_string(l4_kernel_info_t const *k) L4_NOTHROW
{ return (const char *)k + l4_kernel_info_version_offset(k); }

L4_INLINE int
l4_kernel_info_version_offset(l4_kernel_info_t const *kip) L4_NOTHROW
{ return kip->offset_version_strings << 4; }

L4_INLINE l4_cpu_time_t
l4_kip_clock(l4_kernel_info_t const *kip) L4_NOTHROW
{
  // Use kernel-provided code to determine the current clock.
  typedef l4_uint64_t (*kip_time_fn_read_us)(void);
  kip_time_fn_read_us read_us =
    (kip_time_fn_read_us)((l4_uint8_t const*)kip + L4_KIP_OFFS_READ_US);
  return read_us();
}

L4_INLINE l4_cpu_time_t
l4_kip_clock_ns(l4_kernel_info_t const *kip) L4_NOTHROW
{
  typedef l4_uint64_t (*kip_time_fn_read_ns)(void);
  kip_time_fn_read_ns read_ns =
    (kip_time_fn_read_ns)((l4_uint8_t const*)kip + L4_KIP_OFFS_READ_NS);
  return read_ns();
}

L4_INLINE l4_umword_t
l4_kip_clock_lw(l4_kernel_info_t const *kip) L4_NOTHROW
{
  union Clock_field
  {
    l4_cpu_time_t t;
    unsigned long l;
  };
  union Clock_field c = { kip->_clock_val };
  l4_mb();
  return c.l;
}

/**
 * Cycle through kernel features given in the KIP.
 *
 * Cycles through all KIP kernel feature strings. `s` must be a character
 * pointer (`char const *`) initialized with l4_kip_version_string().
 */
#define l4_kip_for_each_feature(s) \
        for (s += __builtin_strlen(s) + 1; *s; s += __builtin_strlen(s) + 1)

/**
 * Check if kernel supports a feature.
 *
 * \param kip  Pointer to the kernel info page (KIP).
 * \param str  Feature name to check.
 *
 * \return  1 if the kernel supports the feature, 0 if not.
 *
 * Checks the feature field in the KIP for the given string.
 */
L4_INLINE int
l4_kip_kernel_has_feature(l4_kernel_info_t const *kip, char const *str)
{
  const char *s = l4_kip_version_string(kip);
  if (!s)
    return 0;

  l4_kip_for_each_feature(s)
    {
      if (__builtin_strcmp(s, str) == 0)
        return 1;
    }

  return 0;
}
