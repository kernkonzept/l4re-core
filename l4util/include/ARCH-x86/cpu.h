/**
 * \file
 * \brief  CPU related functions
 *
 * \author Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef __L4_UTIL_CPU_H
#define __L4_UTIL_CPU_H

#include <l4/sys/compiler.h>

L4_BEGIN_DECLS

/**
 * \defgroup l4util_cpu CPU related functions
 * \ingroup l4util_api
 */
/**@{*/

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

/**@}*/
static inline void
l4util_cpu_pause(void)
{
  __asm__ __volatile__ ("rep; nop");
}

L4_INLINE int
l4util_cpu_has_cpuid(void)
{
  unsigned long eax;

  asm volatile("pushl %%ebx             \t\n"
               "pushfl			\t\n"
               "popl %%eax		\t\n" /* get eflags */
               "movl %%eax, %%ebx	\t\n" /* save it */
               "xorl $0x200000, %%eax	\t\n" /* toggle ID bit */
               "pushl %%eax		\t\n"
               "popfl			\t\n" /* set again */
               "pushfl			\t\n"
               "popl %%eax		\t\n" /* get it again */
               "xorl %%ebx, %%eax	\t\n"
               "pushl %%ebx		\t\n"
               "popfl			\t\n" /* restore saved flags */
               "popl %%ebx              \t\n"
               : "=a" (eax)
               : /* no input */
               );

  return eax & 0x200000;
}

L4_INLINE void
l4util_cpu_cpuid(unsigned long mode,
                 unsigned long *eax, unsigned long *ebx,
                 unsigned long *ecx, unsigned long *edx)
{
  asm volatile("pushl %%ebx      \t\n"
               "cpuid            \t\n"
               "mov %%ebx, %%esi \t\n"
               "popl %%ebx       \t\n"
               : "=a" (*eax),
                 "=S" (*ebx),
                 "=c" (*ecx),
                 "=d" (*edx)
               : "a"  (mode));
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

L4_END_DECLS

#endif

