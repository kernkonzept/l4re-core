/*****************************************************************************/
/**
 * \file
 * \brief   amd64 bit manipulation functions
 * \ingroup bitops
 *
 * \date    07/03/2001
 * \author  Lars Reuther <reuther@os.inf.tu-dresden.de>
 *          Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *          Frank Mehnert <fm3@os.inf.tu-dresden.de> */
/*
 * (c) 2000-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

/*****************************************************************************/
#ifndef __L4UTIL__INCLUDE__ARCH_AMD64__BITOPS_ARCH_H__
#define __L4UTIL__INCLUDE__ARCH_AMD64__BITOPS_ARCH_H__

EXTERN_C_BEGIN

/*****************************************************************************
 *** Implementation
 *****************************************************************************/

#define __L4UTIL_BITOPS_HAVE_ARCH_SET_BIT
L4_INLINE void
l4util_set_bit(int b, volatile l4_umword_t * dest)
{
  __asm__ __volatile__
    (
     "lock; bts  %1,%0   \n\t"
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
     "lock; btr  %1,%0   \n\t"
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
     "lock; btc  %1,%0   \n\t"
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
     "bt    %2,%1   \n\t"
     "setc  %0      \n\t"
     :
     "=r"  (bit)      /* 0,     old bit value */
     :
     "m"   (*dest),   /* 1 mem, destination operand */
     "Ir"  (b)        /* 2,     bit number */
     :
     "memory", "cc"
     );

  return (int)bit;
}


/* bit test and set */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_TEST_AND_SET
L4_INLINE int
l4util_bts(int b, volatile l4_umword_t * dest)
{
  l4_int8_t bit;

  __asm__ __volatile__
    (
     "lock; bts  %2,%1   \n\t"
     "setc  %0      \n\t"
     :
     "=r"  (bit)      /* 0,     old bit value */
     :
     "m"   (*dest),   /* 1 mem, destination operand */
     "Ir"  (b)        /* 2,     bit number */
     :
     "memory", "cc"
     );

  return (int)bit;
}

/* bit test and reset */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_TEST_AND_RESET
L4_INLINE int
l4util_btr(int b, volatile l4_umword_t * dest)
{
  l4_int8_t bit;

  __asm__ __volatile__
    (
     "lock; btr  %2,%1   \n\t"
     "setc  %0      \n\t"
     :
     "=r"  (bit)      /* 0,     old bit value */
     :
     "m"   (*dest),   /* 1 mem, destination operand */
     "Ir"  (b)        /* 2,     bit number */
     :
     "memory", "cc"
     );

  return (int)bit;
}

/* bit test and complement */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_TEST_AND_COMPLEMENT
L4_INLINE int
l4util_btc(int b, volatile l4_umword_t * dest)
{
  l4_int8_t bit;

  __asm__ __volatile__
    (
     "lock; btc   %2,%1   \n\t"
     "setc  %0      \n\t"
     :
     "=r"  (bit)      /* 0,     old bit value */
     :
     "m"   (*dest),   /* 1 mem, destination operand */
     "Ir"  ((l4_umword_t)b)        /* 2,     bit number */
     :
     "memory", "cc"
     );

  return (int)bit;
}

/* bit scan reverse */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_SCAN_REVERSE
L4_INLINE int
l4util_bsr(l4_umword_t word)
{
  l4_umword_t tmp;

  if (L4_UNLIKELY(word == 0))
    return -1;

  __asm__ __volatile__
    (
     "bsr %1,%0 \n\t"
     :
     "=r" (tmp)       /* 0, index of most significant set bit */
     :
     "r"  (word)      /* 1, argument */
     );

  return tmp;
}

/* bit scan forwad */
#define __L4UTIL_BITOPS_HAVE_ARCH_BIT_SCAN_FORWARD
L4_INLINE int
l4util_bsf(l4_umword_t word)
{
  l4_umword_t tmp;

  if (L4_UNLIKELY(word == 0))
    return -1;

  __asm__ __volatile__
    (
     "bsf %1,%0 \n\t"
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
     "xor  %%rax,%%rax		\n\t"
     "repe; scasl		\n\t"
     "jz    1f			\n\t"
     "lea  -4(%%rdi),%%rdi	\n\t"
     "bsfq  (%%rdi),%%rax	\n"
     "1:			\n\t"
     "sub  %%rbx,%%rdi		\n\t"
     "shl  $3,%%rdi		\n\t"
     "add  %%rdi,%%rax		\n\t"
     :
     "=a" (res), "=&c" (dummy0), "=&D" (dummy1)
     :
     "1" ((size + 31) >> 5), "2" (dest), "b" (dest)
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
     "mov   $-1,%%rax		\n\t"
     "xor   %%rdx,%%rdx	\n\t"
     "repe;  scasl		\n\t"
     "je     1f			\n\t"
     "xor   -4(%%rdi),%%rax	\n\t"
     "sub   $4,%%rdi		\n\t"
     "bsf   %%rax,%%rdx	\n"
     "1:			\n\t"
     "sub   %[dest],%%rdi	\n\t"
     "shl   $3,%%rdi		\n\t"
     "add   %%rdi,%%rdx	\n\t"
     :
     "=d" (res), "=&c" (dummy0), "=&D" (dummy1), "=&a" (dummy2)
     :
     "1" ((size + 31) >> 5), "2" (dest), [dest] "S" (dest)
     :
     "cc", "memory");

  return res;
}

EXTERN_C_END

#endif /* ! __L4UTIL__INCLUDE__ARCH_AMD64__BITOPS_ARCH_H__ */
