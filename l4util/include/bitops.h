/*****************************************************************************/
/**
 * \file
 * \brief   bit manipulation functions
 * \ingroup l4util_bitops
 *
 * \date    07/03/2001
 * \author  Lars Reuther <reuther@os.inf.tu-dresden.de> */
/*
 * (c) 2000-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

/*****************************************************************************/
#ifndef __L4UTIL__INCLUDE__BITOPS_H__
#define __L4UTIL__INCLUDE__BITOPS_H__

/* L4 includes */
#include <l4/sys/l4int.h>
#include <l4/sys/compiler.h>

/** define some more usual names */
#define l4util_test_and_clear_bit(b, dest)	l4util_btr(b, dest)
#define l4util_test_and_set_bit(b, dest)	l4util_bts(b, dest)
#define l4util_test_and_change_bit(b, dest)	l4util_btc(b, dest)
#define l4util_log2(word)			l4util_bsr(word)

/*****************************************************************************
 *** Prototypes
 *****************************************************************************/

EXTERN_C_BEGIN

/**
 * \defgroup l4util_bitops Bit Manipulation
 * \ingroup l4util_api
 */

/**
 * \brief Set bit in memory
 * \ingroup l4util_bitops
 *
 * \param  b             bit position
 * \param  dest          destination operand
 */
L4_INLINE void
l4util_set_bit(int b, volatile l4_umword_t * dest);

/**
 * \brief Clear bit in memory
 * \ingroup l4util_bitops
 *
 * \param  b             bit position
 * \param  dest          destination operand
 */
L4_INLINE void
l4util_clear_bit(int b, volatile l4_umword_t * dest);

/**
 * \brief Complement bit in memory
 * \ingroup l4util_bitops
 *
 * \param  b             bit position
 * \param  dest          destination operand
 */
L4_INLINE void
l4util_complement_bit(int b, volatile l4_umword_t * dest);

/**
 * \brief Test bit (return value of bit)
 * \ingroup l4util_bitops
 *
 * \param  b             bit position
 * \param  dest          destination operand
 *
 * \return Value of bit \em b.
 */
L4_INLINE int
l4util_test_bit(int b, const volatile l4_umword_t * dest);

/**
 * \brief Bit test and set
 * \ingroup l4util_bitops
 *
 * \param  b             bit position
 * \param  dest          destination operand
 *
 * \return Old value of bit \em b.
 *
 * Set the \em b bit of \em dest to 1 and return the old value.
 */
L4_INLINE int
l4util_bts(int b, volatile l4_umword_t * dest);

/**
 * \brief Bit test and reset
 * \ingroup l4util_bitops
 *
 * \param  b             bit position
 * \param  dest          destination operand
 *
 * \return Old value of bit \em b.
 *
 * Reset bit \em b and return old value.
 */
L4_INLINE int
l4util_btr(int b, volatile l4_umword_t * dest);

/**
 * \brief Bit test and complement
 * \ingroup l4util_bitops
 *
 * \param  b             bit position
 * \param  dest          destination operand
 *
 * \return Old value of bit \em b.
 *
 * Complement bit \em b and return old value.
 */
L4_INLINE int
l4util_btc(int b, volatile l4_umword_t * dest);

/**
 * \brief Bit scan reverse
 * \ingroup l4util_bitops
 *
 * \param  word          value (machine size)
 *
 * \return index of most significant set bit in word,
 *         -1 if no bit is set (word == 0)
 *
 * "bit scan reverse", find most significant set bit in word (-> LOG2(word))
 */
L4_INLINE int
l4util_bsr(l4_umword_t word);

/**
 * \brief Bit scan forward
 * \ingroup l4util_bitops
 *
 * \param  word          value (machine size)
 *
 * \return index of least significant bit set in word,
 *         -1 if no bit is set (word == 0)
 *
 * "bit scan forward", find least significant bit set in word.
 */
L4_INLINE int
l4util_bsf(l4_umword_t word);

/**
 * \brief Find the first set bit in a memory region
 * \ingroup l4util_bitops
 *
 * \param  dest          bit string
 * \param  size          size of string in bits (must be a multiple of 32!)
 *
 * \return number of the first set bit,
 *         >= size if no bit is set
 */
L4_INLINE int
l4util_find_first_set_bit(const void * dest, l4_size_t size);

/**
 * \brief Find the first zero bit in a memory region
 * \ingroup l4util_bitops
 *
 * \param  dest          bit string
 * \param  size          size of string in bits (must be a multiple of 32!)
 *
 * \return number of the first zero bit,
 *         >= size if no bit is set
 */
L4_INLINE int
l4util_find_first_zero_bit(const void * dest, l4_size_t size);


/**
 * \brief Find the next power of 2 for a given number.
 * \ingroup l4util_bitops
 * 
 * \param val            initial value
 * 
 * \return next-highest power of 2
 */
L4_INLINE int
l4util_next_power2(const unsigned long val);

EXTERN_C_END

/*****************************************************************************
 *** Implementation of specific version
 *****************************************************************************/

#include <l4/util/bitops_arch.h>

/*****************************************************************************
 *** Generic implementations
 *****************************************************************************/

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_SET_BIT
#include <l4/util/atomic.h>
L4_INLINE void
l4util_set_bit(int b, volatile l4_umword_t * dest)
{
  l4_umword_t oldval, newval;

  dest += b / (sizeof(*dest) * 8); /* advance dest to the proper element */
  b    &= sizeof(*dest) * 8 - 1;   /* modulo; cut off all upper bits */

  do
    {
      oldval = *dest;
      newval = oldval | (1UL << b);
    }
  while (!l4util_cmpxchg(dest, oldval, newval));
}
#endif

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_CLEAR_BIT
#include <l4/util/atomic.h>
L4_INLINE void
l4util_clear_bit(int b, volatile l4_umword_t * dest)
{
  l4_umword_t oldval, newval;

  dest += b / (sizeof(*dest) * 8);
  b    &= sizeof(*dest) * 8 - 1;

  do
    {
      oldval = *dest;
      newval = oldval & ~(1UL << b);
    }
  while (!l4util_cmpxchg(dest, oldval, newval));
}
#endif

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_TEST_BIT
L4_INLINE int
l4util_test_bit(int b, const volatile l4_umword_t * dest)
{
  dest += b / (sizeof(*dest) * 8);
  b    &= sizeof(*dest) * 8 - 1;

  return (*dest >> b) & 1;
}
#endif

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_BIT_TEST_AND_SET
#include <l4/util/atomic.h>
L4_INLINE int
l4util_bts(int b, volatile l4_umword_t * dest)
{
  l4_umword_t oldval, newval;

  dest += b / (sizeof(*dest) * 8);
  b    &= sizeof(*dest) * 8 - 1;

  do
    {
      oldval = *dest;
      newval = oldval | (1UL << b);
    }
  while (!l4util_cmpxchg(dest, oldval, newval));

  /* Return old bit */
  return (oldval >> b) & 1;
}
#endif

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_BIT_TEST_AND_RESET
#include <l4/util/atomic.h>
L4_INLINE int
l4util_btr(int b, volatile l4_umword_t * dest)
{
  l4_umword_t oldval, newval;

  dest += b / (sizeof(*dest) * 8);
  b    &= sizeof(*dest) * 8 - 1;

  do
    {
      oldval = *dest;
      newval = oldval & ~(1UL << b);
    }
  while (!l4util_cmpxchg(dest, oldval, newval));

  /* Return old bit */
  return (oldval >> b) & 1;
}
#endif

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_BIT_SCAN_REVERSE
L4_INLINE int
l4util_bsr(l4_umword_t word)
{
  int i;

  if (!word)
    return -1;

  for (i = 8 * sizeof(word) - 1; i >= 0; i--)
    if ((1UL << i) & word)
      return i;

  return -1;
}
#endif

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_BIT_SCAN_FORWARD
L4_INLINE int
l4util_bsf(l4_umword_t word)
{
  unsigned int i;

  if (!word)
    return -1;

  for (i = 0; i < sizeof(word) * 8; i++)
    if ((1UL << i) & word)
      return i;

  return -1;
}
#endif

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_FIND_FIRST_ZERO_BIT
L4_INLINE int
l4util_find_first_zero_bit(const void * dest, l4_size_t size)
{
  l4_size_t i, j;
  unsigned long *v = (unsigned long*)dest;

  if (!size)
    return 0;

  size = (size + 31) & ~0x1f; /* Grmbl: adapt to x86 implementation... */

  for (i = j = 0; i < size; i++, j++)
    {
      if (j >= sizeof(*v) * 8)
	{
	  j = 0;
	  v++;
	}
      if (!((1UL << j) & *v))
	return i;
    }
  return size + 1;
}
#endif

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_COMPLEMENT_BIT
L4_INLINE void
l4util_complement_bit(int b, volatile l4_umword_t * dest)
{
  dest += b / (sizeof(*dest) * 8);
  b    &= sizeof(*dest) * 8 - 1;

  *dest ^= 1UL << b;
}
#endif

/*
 * Adapted from:
 * http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_find_the_next-highest_power_of_two
 */
L4_INLINE int
l4util_next_power2(unsigned long val)
{
  unsigned i;

  if (val == 0)
    return 1;

  val--;
  for (i=1; i < sizeof(unsigned long)*8; i<<=1)
    val = val | val >> i;

  return val+1;
}


/* Non-implemented version, catch with a linker warning */

extern int __this_l4util_bitops_function_is_not_implemented_for_this_arch__sorry(void);

#ifndef __L4UTIL_BITOPS_HAVE_ARCH_BIT_TEST_AND_COMPLEMENT
L4_INLINE int
l4util_btc(int b, volatile l4_umword_t * dest)
{ (void)b; (void)dest; __this_l4util_bitops_function_is_not_implemented_for_this_arch__sorry(); return 0; }
#endif

#ifndef  __L4UTIL_BITOPS_HAVE_ARCH_FIND_FIRST_SET_BIT
L4_INLINE int
l4util_find_first_set_bit(const void * dest, l4_size_t size)
{ (void)dest; (void)size; __this_l4util_bitops_function_is_not_implemented_for_this_arch__sorry(); return 0; }
#endif

#endif /* ! __L4UTIL__INCLUDE__BITOPS_H__ */
