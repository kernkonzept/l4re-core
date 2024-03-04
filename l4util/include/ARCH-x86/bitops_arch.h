/* SPDX-License-Identifier: GPL-2.0-only or License-Ref-kk-custom */
/*
 * Copyright (C) 2000-2009 Technische Universität Dresden (Germany)
 * Copyright (C) 2016, 2022 Kernkonzept GmbH. All rights reserved.
 * Author(s): Lars Reuther <reuther@os.inf.tu-dresden.de>
 *            Frank Mehnert <frank.mehnert@kernkonzept.com>
 */

/**
 * \file
 * x86 bit manipulation functions
 * \ingroup l4util_bitops
 */

#pragma once

EXTERN_C_BEGIN

/* set bit */
#define __L4UTIL_BITOPS_HAVE_ARCH_SET_BIT
L4_INLINE void
l4util_set_bit(int b, volatile l4_umword_t * dest)
{
  __asm__ __volatile__
    (
     "lock; btsl  %1,%0   \n\t"
     :
     :
     "m"   (*dest),   /* 0 mem, destination operand */
     "Ir"  (b)       /* 1,     bit number */
     :
     "memory", "cc"
     );
}

/* clear bit */
#define __L4UTIL_BITOPS_HAVE_ARCH_CLEAR_BIT
L4_INLINE void
l4util_clear_bit(int b, volatile l4_umword_t * dest)
{
  __asm__ __volatile__
    (
     "lock; btrl  %1,%0   \n\t"
     :
     :
     "m"   (*dest),   /* 0 mem, destination operand */
     "Ir"  (b)        /* 1,     bit number */
     :
     "memory", "cc"
     );
}

/* change bit */
#define __L4UTIL_BITOPS_HAVE_ARCH_COMPLEMENT_BIT
L4_INLINE void
l4util_complement_bit(int b, volatile l4_umword_t * dest)
{
  __asm__ __volatile__
    (
     "lock; btcl  %1,%0   \n\t"
     :
     :
     "m"   (*dest),   /* 0 mem, destination operand */
     "Ir"  (b)        /* 1,     bit number */
     :
     "memory", "cc"
     );
}

/* test bit */
#define __L4UTIL_BITOPS_HAVE_ARCH_TEST_BIT
L4_INLINE int
l4util_test_bit(int b, const volatile l4_umword_t * dest)
{
  l4_int8_t bit;

  __asm__ __volatile__
    (
     "btl   %2,%1   \n\t"
     :
     "=@ccc" (bit)    /* 0,     old bit value */
     :
     "m"   (*dest),   /* 1 mem, destination operand */
     "Ir"  (b)        /* 2,     bit number */
     :
     "memory"
     );

  return bit;
}

/* bit test and set */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_TEST_AND_SET
L4_INLINE int
l4util_bts(int b, volatile l4_umword_t * dest)
{
  l4_int8_t bit;

  __asm__ __volatile__
    (
     "lock; btsl  %2,%1   \n\t"
     :
     "=@ccc" (bit)    /* 0,     old bit value */
     :
     "m"   (*dest),   /* 1 mem, destination operand */
     "Ir"  (b)        /* 2,     bit number */
     :
     "memory"
     );

  return bit;
}

/* bit test and reset */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_TEST_AND_RESET
L4_INLINE int
l4util_btr(int b, volatile l4_umword_t * dest)
{
  l4_int8_t bit;

  __asm__ __volatile__
    (
     "lock; btrl  %2,%1   \n\t"
     :
     "=@ccc" (bit)    /* 0,     old bit value */
     :
     "m"   (*dest),   /* 1 mem, destination operand */
     "Ir"  (b)        /* 2,     bit number */
     :
     "memory"
     );

  return bit;
}

/* bit test and complement */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_TEST_AND_COMPLEMENT
L4_INLINE int
l4util_btc(int b, volatile l4_umword_t * dest)
{
  l4_int8_t bit;

  __asm__ __volatile__
    (
     "lock; btcl  %2,%1   \n\t"
     :
     "=@ccc" (bit)    /* 0,     old bit value */
     :
     "m"   (*dest),   /* 1 mem, destination operand */
     "Ir"  (b)        /* 2,     bit number */
     :
     "memory"
     );

  return bit;
}

/* bit scan reverse */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_SCAN_REVERSE
L4_INLINE int
l4util_bsr(l4_umword_t word)
{
  int tmp;

  if (L4_UNLIKELY(word == 0))
    return -1;

  __asm__ __volatile__
    (
     "bsrl %1,%0 \n\t"
     :
     "=r" (tmp)       /* 0, index of most significant set bit */
     :
     "r"  (word)      /* 1, argument */
     );

  return tmp;
}

/* bit scan forward */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_SCAN_FORWARD
L4_INLINE int
l4util_bsf(l4_umword_t word)
{
  int tmp;

  if (L4_UNLIKELY(word == 0))
    return -1;

  __asm__ __volatile__
    (
     "bsfl %1,%0 \n\t"
     :
     "=r" (tmp)       /* 0, index of least significant set bit */
     :
     "r"  (word)      /* 1, argument */
     );

  return tmp;
}

#define __L4UTIL_BITOPS_HAVE_ARCH_FIND_FIRST_SET_BIT
L4_INLINE int
l4util_find_first_set_bit(const void * dest, l4_size_t size)
{
  l4_mword_t dummy0, dummy1, res;

  __asm__ __volatile__
    (
     "repe; scasl               \n\t"
     "jz    1f                  \n\t"
     "lea  -4(%%edi),%%edi      \n\t"
     "bsf  (%%edi),%%eax        \n"
     "1:                        \n\t"
     "sub  %%esi,%%edi          \n\t"
     "shl  $3,%%edi             \n\t"
     "add  %%edi,%%eax          \n\t"
     :
     "=a" (res), "=c" (dummy0), "=D" (dummy1)
     :
     "a" (0), "c" ((size + 31) >> 5), "D" (dest), "S" (dest)
     :
     "cc", "memory");

  return res;
}

#define __L4UTIL_BITOPS_HAVE_ARCH_FIND_FIRST_ZERO_BIT
L4_INLINE int
l4util_find_first_zero_bit(const void * dest, l4_size_t size)
{
  l4_mword_t dummy0, dummy1, dummy2, res;

  if (!size)
    return 0;

  __asm__ __volatile__
    (
     "repe;  scasl              \n\t"
     "je     1f                 \n\t"
     "xor   -4(%%edi),%%eax     \n\t"
     "sub   $4,%%edi            \n\t"
     "bsf   %%eax,%%edx         \n"
     "1:                        \n\t"
     "sub   %%esi,%%edi         \n\t"
     "shl   $3,%%edi            \n\t"
     "add   %%edi,%%edx         \n\t"
     :
     "=a" (dummy0), "=c" (dummy1), "=d" (res), "=D" (dummy2)
     :
     "a" (~0UL), "c" ((size + 31) >> 5), "d" (0), "D" (dest), "S" (dest)
     :
     "cc", "memory");

  return res;
}

EXTERN_C_END
