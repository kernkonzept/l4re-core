/**
 * \file
 * \brief  Cache functions
 *
 * \date   2007-11
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2007-2009 Author(s)
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
#ifndef __L4SYS__INCLUDE__ARCH_ARM__CACHE_H__
#define __L4SYS__INCLUDE__ARCH_ARM__CACHE_H__

#include <l4/sys/compiler.h>

#include_next <l4/sys/cache.h>

L4_INLINE unsigned long __attribute__((pure, always_inline))
l4_cache_arm_ctr(void);

L4_INLINE unsigned long __attribute__((pure, always_inline))
l4_cache_arm_ctr(void)
{
  unsigned long v;
  asm ("mrs %0, CTR_EL0" : "=r"(v));
  return v;
}

L4_INLINE unsigned __attribute__((pure, always_inline))
l4_cache_dmin_line(void);

L4_INLINE unsigned __attribute__((pure, always_inline))
l4_cache_dmin_line(void)
{
  return 4U << ((l4_cache_arm_ctr() >> 16) & 0xf);
}

#define L4_ARM_CACHE_LOOP(op) \
  if (start > end)            \
    __builtin_unreachable();  \
                              \
  unsigned long const sz = end - start;                     \
  unsigned const s = l4_cache_dmin_line();                  \
  asm volatile ("dsb ish" : : "m"(*((unsigned *)start)));   \
  unsigned i;                                               \
  unsigned long x = start;                                  \
  for (i = 0; i < sz; i += s)                               \
    {                                                       \
      asm (op ", %1" : "+m"(*((unsigned *)x)) : "r"(x)); \
      x += s;                                               \
    }                                                       \
  asm volatile ("dsb ish");


L4_INLINE int
l4_cache_clean_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  L4_ARM_CACHE_LOOP("dc cvac");
  return 0;
}

L4_INLINE int
l4_cache_flush_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  L4_ARM_CACHE_LOOP("dc civac");
  return 0;
}

L4_INLINE int
l4_cache_inv_data(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  // DC IVAC is always privileged, use DC CIVAC instead
  L4_ARM_CACHE_LOOP("dc civac");
  return 0;
}

L4_INLINE int
l4_cache_coherent(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  L4_ARM_CACHE_LOOP("dc cvau, %1; ic ivau");
  return 0;
}

L4_INLINE int
l4_cache_dma_coherent(unsigned long start,
                      unsigned long end) L4_NOTHROW
{
  L4_ARM_CACHE_LOOP("dc civac");
  return 0;
}

#undef L4_ARM_CACHE_LOOP

#endif /* ! __L4SYS__INCLUDE__ARCH_ARM__CACHE_H__ */
