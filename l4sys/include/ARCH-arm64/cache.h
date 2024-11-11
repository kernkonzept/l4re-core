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
 * License: see LICENSE.spdx (in this directory or the directories above)
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

#define L4_ARM_CACHE_LOOP(op)                                  \
  unsigned long step;                                          \
                                                               \
  if (start > end)                                             \
    __builtin_unreachable();                                   \
                                                               \
  step = l4_cache_dmin_line();                                 \
  start &= ~(step - 1);                                        \
  end = (end + step - 1) & ~(step - 1);                        \
  for (; start != end; start += step)                          \
    asm volatile (op ", %0" : : "r"(start) : "memory");        \
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
  L4_ARM_CACHE_LOOP("dc cvau, %0; ic ivau");
  asm volatile ("isb");
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
