/*****************************************************************************/
/**
 * \file
 * \brief   atomic operations header and generic implementations
 * \ingroup l4util_atomic
 *
 * \date    10/20/2000
 * \author  Lars Reuther <reuther@os.inf.tu-dresden.de>,
 *          Jork Loeser  <jork@os.inf.tu-dresden.de> */
/*
 * (c) 2000-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

/*****************************************************************************/
#ifndef __L4UTIL__INCLUDE__ATOMIC_H__
#define __L4UTIL__INCLUDE__ATOMIC_H__

#include <l4/sys/l4int.h>
#include <l4/sys/compiler.h>

/*****************************************************************************
 *** Prototypes
 *****************************************************************************/

EXTERN_C_BEGIN

/** 
 * \defgroup l4util_atomic Atomic Instructions
 * \ingroup l4util_api
 */

/**
 * \brief Atomic compare and exchange (64 bit version)
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  cmp_val       compare value
 * \param  new_val       new value for dest
 *
 * \return 0 if comparison failed, 1 otherwise
 *
 * Compare the value in \em dest with \em cmp_val, if equal set \em dest to
 * \em new_val
 */
L4_INLINE int
l4util_cmpxchg64(volatile l4_uint64_t * dest,
                 l4_uint64_t cmp_val, l4_uint64_t new_val);

/**
 * \brief Atomic compare and exchange (32 bit version)
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  cmp_val       compare value
 * \param  new_val       new value for dest
 *
 * \return 0 if comparison failed, !=0 otherwise
 *
 * Compare the value in \em dest with \em cmp_val, if equal set \em dest to
 * \em new_val
 */
L4_INLINE int
l4util_cmpxchg32(volatile l4_uint32_t * dest,
                 l4_uint32_t cmp_val, l4_uint32_t new_val);

/**
 * \brief Atomic compare and exchange (16 bit version)
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  cmp_val       compare value
 * \param  new_val       new value for dest
 *
 * \return 0 if comparison failed, !=0 otherwise
 *
 * Compare the value in \em dest with \em cmp_val, if equal set \em dest to
 * \em new_val
 */
L4_INLINE int
l4util_cmpxchg16(volatile l4_uint16_t * dest,
                 l4_uint16_t cmp_val, l4_uint16_t new_val);

/**
 * \brief Atomic compare and exchange (8 bit version)
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  cmp_val       compare value
 * \param  new_val       new value for dest
 *
 * \return 0 if comparison failed, !=0 otherwise
 *
 * Compare the value in \em dest with \em cmp_val, if equal set \em dest to
 * \em new_val
 */
L4_INLINE int
l4util_cmpxchg8(volatile l4_uint8_t * dest,
                l4_uint8_t cmp_val, l4_uint8_t new_val);

/**
 * \brief Atomic compare and exchange (machine wide fields)
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  cmp_val       compare value
 * \param  new_val       new value for dest
 *
 * \return 0 if comparison failed, 1 otherwise
 *
 * Compare the value in \em dest with \em cmp_val, if equal set \em dest to
 * \em new_val
 */
L4_INLINE int
l4util_cmpxchg(volatile l4_umword_t * dest,
               l4_umword_t cmp_val, l4_umword_t new_val);

/**
 * \brief Atomic exchange (32 bit version)
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  val           new value for dest
 *
 * \return old value at destination
 */
L4_INLINE l4_uint32_t
l4util_xchg32(volatile l4_uint32_t * dest, l4_uint32_t val);

/**
 * \brief Atomic exchange (16 bit version)
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  val           new value for dest
 *
 * \return old value at destination
 */
L4_INLINE l4_uint16_t
l4util_xchg16(volatile l4_uint16_t * dest, l4_uint16_t val);

/**
 * \brief Atomic exchange (8 bit version)
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  val           new value for dest
 *
 * \return old value at destination
 */
L4_INLINE l4_uint8_t
l4util_xchg8(volatile l4_uint8_t * dest, l4_uint8_t val);

/**
 * \brief Atomic exchange (machine wide fields)
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  val           new value for dest
 *
 * \return old value at destination
 */
L4_INLINE l4_umword_t
l4util_xchg(volatile l4_umword_t * dest, l4_umword_t val);

//!@name Atomic add/sub/and/or (8,16,32 bit version) without result
/** @{
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  val           value to add/sub/and/or
 */
L4_INLINE void
l4util_add8(volatile l4_uint8_t *dest, l4_uint8_t val);
L4_INLINE void
l4util_add16(volatile l4_uint16_t *dest, l4_uint16_t val);
L4_INLINE void
l4util_add32(volatile l4_uint32_t *dest, l4_uint32_t val);
L4_INLINE void
l4util_sub8(volatile l4_uint8_t *dest, l4_uint8_t val);
L4_INLINE void
l4util_sub16(volatile l4_uint16_t *dest, l4_uint16_t val);
L4_INLINE void
l4util_sub32(volatile l4_uint32_t *dest, l4_uint32_t val);
L4_INLINE void
l4util_and8(volatile l4_uint8_t *dest, l4_uint8_t val);
L4_INLINE void
l4util_and16(volatile l4_uint16_t *dest, l4_uint16_t val);
L4_INLINE void
l4util_and32(volatile l4_uint32_t *dest, l4_uint32_t val);
L4_INLINE void
l4util_or8(volatile l4_uint8_t *dest, l4_uint8_t val);
L4_INLINE void
l4util_or16(volatile l4_uint16_t *dest, l4_uint16_t val);
L4_INLINE void
l4util_or32(volatile l4_uint32_t *dest, l4_uint32_t val);
//@}

//!@name Atomic add/sub/and/or operations (8,16,32 bit) with result
/** @{
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \param  val           value to add/sub/and/or
 * \return res
 */
L4_INLINE l4_uint8_t
l4util_add8_res(volatile l4_uint8_t *dest, l4_uint8_t val);
L4_INLINE l4_uint16_t
l4util_add16_res(volatile l4_uint16_t *dest, l4_uint16_t val);
L4_INLINE l4_uint32_t
l4util_add32_res(volatile l4_uint32_t *dest, l4_uint32_t val);
L4_INLINE l4_uint8_t
l4util_sub8_res(volatile l4_uint8_t *dest, l4_uint8_t val);
L4_INLINE l4_uint16_t
l4util_sub16_res(volatile l4_uint16_t *dest, l4_uint16_t val);
L4_INLINE l4_uint32_t
l4util_sub32_res(volatile l4_uint32_t *dest, l4_uint32_t val);
L4_INLINE l4_uint8_t
l4util_and8_res(volatile l4_uint8_t *dest, l4_uint8_t val);
L4_INLINE l4_uint16_t
l4util_and16_res(volatile l4_uint16_t *dest, l4_uint16_t val);
L4_INLINE l4_uint32_t
l4util_and32_res(volatile l4_uint32_t *dest, l4_uint32_t val);
L4_INLINE l4_uint8_t
l4util_or8_res(volatile l4_uint8_t *dest, l4_uint8_t val);
L4_INLINE l4_uint16_t
l4util_or16_res(volatile l4_uint16_t *dest, l4_uint16_t val);
L4_INLINE l4_uint32_t
l4util_or32_res(volatile l4_uint32_t *dest, l4_uint32_t val);
//@}

//!@name Atomic inc/dec (8,16,32 bit) without result
/** @{
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 */
L4_INLINE void
l4util_inc8(volatile l4_uint8_t *dest);
L4_INLINE void
l4util_inc16(volatile l4_uint16_t *dest);
L4_INLINE void
l4util_inc32(volatile l4_uint32_t *dest);
L4_INLINE void
l4util_dec8(volatile l4_uint8_t *dest);
L4_INLINE void
l4util_dec16(volatile l4_uint16_t *dest);
L4_INLINE void
l4util_dec32(volatile l4_uint32_t *dest);
//@}

//!@name Atomic inc/dec (8,16,32 bit) with result
/** @{
 * \ingroup l4util_atomic
 *
 * \param  dest          destination operand
 * \return res
 */
L4_INLINE l4_uint8_t
l4util_inc8_res(volatile l4_uint8_t *dest);
L4_INLINE l4_uint16_t
l4util_inc16_res(volatile l4_uint16_t *dest);
L4_INLINE l4_uint32_t
l4util_inc32_res(volatile l4_uint32_t *dest);
L4_INLINE l4_uint8_t
l4util_dec8_res(volatile l4_uint8_t *dest);
L4_INLINE l4_uint16_t
l4util_dec16_res(volatile l4_uint16_t *dest);
L4_INLINE l4_uint32_t
l4util_dec32_res(volatile l4_uint32_t *dest);
//@}

/**
 * \brief Atomic add
 * \ingroup l4util_atomic
 *
 * \param  dest      destination operand
 * \param  val       value to add
 */
L4_INLINE void
l4util_atomic_add(volatile long *dest, long val);

/**
 * \brief Atomic increment
 * \ingroup l4util_atomic
 *
 * \param  dest      destination operand
 */
L4_INLINE void
l4util_atomic_inc(volatile long *dest);

EXTERN_C_END

/*****************
 * IMPLEMENTAION *
 *****************/

L4_INLINE int
l4util_cmpxchg64(volatile l4_uint64_t * dest,
                 l4_uint64_t cmp_val, l4_uint64_t new_val)
{
  return __atomic_compare_exchange_n(dest, &cmp_val, new_val, 0,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

L4_INLINE int
l4util_cmpxchg32(volatile l4_uint32_t * dest,
                 l4_uint32_t cmp_val, l4_uint32_t new_val)
{
  return __atomic_compare_exchange_n(dest, &cmp_val, new_val, 0,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

L4_INLINE int
l4util_cmpxchg16(volatile l4_uint16_t * dest,
                 l4_uint16_t cmp_val, l4_uint16_t new_val)
{
  return __atomic_compare_exchange_n(dest, &cmp_val, new_val, 0,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

L4_INLINE int
l4util_cmpxchg8(volatile l4_uint8_t * dest,
                l4_uint8_t cmp_val, l4_uint8_t new_val)
{
  return __atomic_compare_exchange_n(dest, &cmp_val, new_val, 0,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

L4_INLINE int
l4util_cmpxchg(volatile l4_umword_t * dest,
               l4_umword_t cmp_val, l4_umword_t new_val)
{
  return __atomic_compare_exchange_n(dest, &cmp_val, new_val, 0,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

L4_INLINE l4_uint32_t
l4util_xchg32(volatile l4_uint32_t * dest, l4_uint32_t val)
{
  return __atomic_exchange_n(dest, val, __ATOMIC_SEQ_CST);
}

L4_INLINE l4_uint16_t
l4util_xchg16(volatile l4_uint16_t * dest, l4_uint16_t val)
{
  return __atomic_exchange_n(dest, val, __ATOMIC_SEQ_CST);
}

L4_INLINE l4_uint8_t
l4util_xchg8(volatile l4_uint8_t * dest, l4_uint8_t val)
{
  return __atomic_exchange_n(dest, val, __ATOMIC_SEQ_CST);
}

L4_INLINE l4_umword_t
l4util_xchg(volatile l4_umword_t * dest, l4_umword_t val)
{
  return __atomic_exchange_n(dest, val, __ATOMIC_SEQ_CST);
}

L4_INLINE void
l4util_inc8(volatile l4_uint8_t *dest)
{ __atomic_fetch_add(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_inc16(volatile l4_uint16_t *dest)
{ __atomic_fetch_add(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_inc32(volatile l4_uint32_t *dest)
{ __atomic_fetch_add(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_atomic_inc(volatile long *dest)
{ __atomic_fetch_add(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_dec8(volatile l4_uint8_t *dest)
{ __atomic_fetch_sub(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_dec16(volatile l4_uint16_t *dest)
{ __atomic_fetch_sub(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_dec32(volatile l4_uint32_t *dest)
{ __atomic_fetch_sub(dest, 1, __ATOMIC_SEQ_CST); }


L4_INLINE l4_uint8_t
l4util_inc8_res(volatile l4_uint8_t *dest)
{ return __atomic_add_fetch(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint16_t
l4util_inc16_res(volatile l4_uint16_t *dest)
{ return __atomic_add_fetch(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint32_t
l4util_inc32_res(volatile l4_uint32_t *dest)
{ return __atomic_add_fetch(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint8_t
l4util_dec8_res(volatile l4_uint8_t *dest)
{ return __atomic_sub_fetch(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint16_t
l4util_dec16_res(volatile l4_uint16_t *dest)
{ return __atomic_sub_fetch(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint32_t
l4util_dec32_res(volatile l4_uint32_t *dest)
{ return __atomic_sub_fetch(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE l4_umword_t
l4util_dec_res(volatile l4_umword_t *dest)
{ return __atomic_sub_fetch(dest, 1, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_add8(volatile l4_uint8_t *dest, l4_uint8_t val)
{ __atomic_fetch_add(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_add16(volatile l4_uint16_t *dest, l4_uint16_t val)
{ __atomic_fetch_add(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_add32(volatile l4_uint32_t *dest, l4_uint32_t val)
{ __atomic_fetch_add(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_atomic_add(volatile long *dest, long val)
{ __atomic_fetch_add(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_sub8(volatile l4_uint8_t *dest, l4_uint8_t val)
{ __atomic_fetch_sub(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_sub16(volatile l4_uint16_t *dest, l4_uint16_t val)
{ __atomic_fetch_sub(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_sub32(volatile l4_uint32_t *dest, l4_uint32_t val)
{ __atomic_fetch_sub(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_and8(volatile l4_uint8_t *dest, l4_uint8_t val)
{ __atomic_fetch_and(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_and16(volatile l4_uint16_t *dest, l4_uint16_t val)
{ __atomic_fetch_and(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_and32(volatile l4_uint32_t *dest, l4_uint32_t val)
{ __atomic_fetch_and(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_or8(volatile l4_uint8_t *dest, l4_uint8_t val)
{ __atomic_fetch_or(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_or16(volatile l4_uint16_t *dest, l4_uint16_t val)
{ __atomic_fetch_or(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE void
l4util_or32(volatile l4_uint32_t *dest, l4_uint32_t val)
{ __atomic_fetch_or(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint8_t
l4util_add8_res(volatile l4_uint8_t *dest, l4_uint8_t val)
{ return __atomic_add_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint16_t
l4util_add16_res(volatile l4_uint16_t *dest, l4_uint16_t val)
{ return __atomic_add_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint32_t
l4util_add32_res(volatile l4_uint32_t *dest, l4_uint32_t val)
{ return __atomic_add_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint8_t
l4util_sub8_res(volatile l4_uint8_t *dest, l4_uint8_t val)
{ return __atomic_sub_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint16_t
l4util_sub16_res(volatile l4_uint16_t *dest, l4_uint16_t val)
{ return __atomic_sub_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint32_t
l4util_sub32_res(volatile l4_uint32_t *dest, l4_uint32_t val)
{ return __atomic_sub_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint8_t
l4util_and8_res(volatile l4_uint8_t *dest, l4_uint8_t val)
{ return __atomic_and_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint16_t
l4util_and16_res(volatile l4_uint16_t *dest, l4_uint16_t val)
{ return __atomic_and_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint32_t
l4util_and32_res(volatile l4_uint32_t *dest, l4_uint32_t val)
{ return __atomic_and_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint8_t
l4util_or8_res(volatile l4_uint8_t *dest, l4_uint8_t val)
{ return __atomic_or_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint16_t
l4util_or16_res(volatile l4_uint16_t *dest, l4_uint16_t val)
{ return __atomic_or_fetch(dest, val, __ATOMIC_SEQ_CST); }

L4_INLINE l4_uint32_t
l4util_or32_res(volatile l4_uint32_t *dest, l4_uint32_t val)
{ return __atomic_or_fetch(dest, val, __ATOMIC_SEQ_CST); }

#endif /* ! __L4UTIL__INCLUDE__ATOMIC_H__ */
