/**
 * \file
 * \brief  CPU related functions
 *
 * \author Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef __L4_UTIL_CPU_H
#define __L4_UTIL_CPU_H

#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

/**
 * \defgroup l4util_cpu CPU related functions
 * \ingroup l4util_api
 */
/*@{*/

/**
 * Check whether the CPU supports the "cpuid" instruction.
 *
 * \return 1 if it has, 0 if it has not
 */
L4_INLINE int          l4util_cpu_has_cpuid(void);

/**
 * Returns the CPU capabilities if the "cpuid" instruction is available.
 *
 * \return CPU capabilities if the "cpuid" instruction is available,
 *         0 if the "cpuid" instruction is not supported.
 */
L4_INLINE unsigned int l4util_cpu_capabilities(void);

/**
 * Returns the CPU capabilities.
 *
 * \return CPU capabilities.
 */
L4_INLINE unsigned int l4util_cpu_capabilities_nocheck(void);

/**
 * Generic CPUID access function.
 */
L4_INLINE void
l4util_cpu_cpuid(unsigned long mode,
                 unsigned long *eax, unsigned long *ebx,
                 unsigned long *ecx, unsigned long *edx);

/*@}*/
static inline void
l4util_cpu_pause(void)
{
  __asm__ __volatile__ ("rep; nop");
}

L4_INLINE int
l4util_cpu_has_cpuid(void)
{
  return 1;
}

L4_INLINE void
l4util_cpu_cpuid(unsigned long mode,
                 unsigned long *eax, unsigned long *ebx,
                 unsigned long *ecx, unsigned long *edx)
{
  asm volatile("cpuid"
               : "=a" (*eax),
                 "=b" (*ebx),
                 "=c" (*ecx),
                 "=d" (*edx)
               : "a"  (mode)
               );
}

L4_INLINE unsigned int
l4util_cpu_capabilities_nocheck(void)
{
  unsigned long dummy, capability;

  /* get CPU capabilities */
  l4util_cpu_cpuid(1, &dummy, &dummy, &dummy, &capability);

  return capability;
}

L4_INLINE unsigned int
l4util_cpu_capabilities(void)
{
  if (!l4util_cpu_has_cpuid())
    return 0; /* CPU has not cpuid instruction */

  return l4util_cpu_capabilities_nocheck();
}

EXTERN_C_END

#endif

