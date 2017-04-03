/* Copyright (C) 2010-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _BITS_ATOMIC_H
#define _BITS_ATOMIC_H	1

#include <stdint.h>

/* This is needed to break a depencency loop, we do not need errno anyway */
#ifndef _ERRNO_H
# define _ERRNO_H
# include <sysdep.h>
# undef _ERRNO_H
#else
# include <sysdep.h>
#endif

/* Or1k has no atomic compare-and-exchange operation, but the
   kernel provides userspace atomicity operations.  Use them.  */

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

/* TODO: Move these to a kernel header */
#define OR1K_ATOMIC_SWAP	1
#define OR1K_ATOMIC_CMPXCHG	2
#define OR1K_ATOMIC_XCHG	3
#define OR1K_ATOMIC_ADD		4
#define OR1K_ATOMIC_DECPOS	5
#define OR1K_ATOMIC_AND		6
#define OR1K_ATOMIC_OR		7
#define OR1K_ATOMIC_UMAX	8
#define OR1K_ATOMIC_UMIN	9

#define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  ((__typeof (*(mem))) ((sizeof (*(mem)) == 4) ? \
	 INTERNAL_SYSCALL (or1k_atomic, , 4, \
	    OR1K_ATOMIC_CMPXCHG, mem, oldval, newval) \
	 : __atomic_error_bad_argument_size ()))

#define atomic_exchange_acq(mem, newval) \
  ((__typeof (*(mem))) ((sizeof (*(mem)) == 4) ? \
	 INTERNAL_SYSCALL (or1k_atomic, , 3, \
	    OR1K_ATOMIC_XCHG, mem, newval) \
	 : __atomic_error_bad_argument_size ()))

#define atomic_exchange_and_add_acq(mem, val) \
  ((__typeof (*(mem))) ((sizeof (*(mem)) == 4) ? \
	 INTERNAL_SYSCALL (or1k_atomic, , 3, \
	    OR1K_ATOMIC_ADD, mem, val) \
	 : __atomic_error_bad_argument_size ()))

#define atomic_decrement_if_positive(mem) \
  ((__typeof (*(mem))) ((sizeof (*(mem)) == 4) ? \
	 INTERNAL_SYSCALL (or1k_atomic, , 2, \
	    OR1K_ATOMIC_DECPOS, mem) \
	 : __atomic_error_bad_argument_size ()))

#define atomic_and_val(mem, mask) \
  ((__typeof (*(mem))) ((sizeof (*(mem)) == 4) ? \
	 INTERNAL_SYSCALL (or1k_atomic, , 3, \
	    OR1K_ATOMIC_AND, mem, mask) \
	 : __atomic_error_bad_argument_size ()))

#define atomic_or_val(mem, mask) \
  ((__typeof (*(mem))) ((sizeof (*(mem)) == 4) ? \
	 INTERNAL_SYSCALL (or1k_atomic, , 3, \
	    OR1K_ATOMIC_OR, mem, mask) \
	 : __atomic_error_bad_argument_size ()))

#define atomic_max_val(mem, val) \
  ((__typeof (*(mem))) ((sizeof (*(mem)) == 4) ? \
	 INTERNAL_SYSCALL (or1k_atomic, , 3, \
	    OR1K_ATOMIC_UMAX, mem, val) \
	 : __atomic_error_bad_argument_size ()))

#define atomic_min_val(mem, val) \
  ((__typeof (*(mem))) ((sizeof (*(mem)) == 4) ? \
	 INTERNAL_SYSCALL (or1k_atomic, , 3, \
	    OR1K_ATOMIC_UMIN, mem, val) \
	 : __atomic_error_bad_argument_size ()))


/* atomic_bit_test_set in terms of atomic_or_val. */
#define atomic_bit_test_set(mem, bit)                                    \
    ({ __typeof (*(mem)) __att0_mask = ((__typeof (*(mem))) 1 << (bit)); \
         atomic_or_val ((mem), __att0_mask) & __att0_mask; })

/* Various macros that should just be synonyms. */
#define catomic_exchange_and_add atomic_exchange_and_add
#define atomic_and(mem, mask) ((void) atomic_and_val ((mem), (mask)))
#define catomic_and atomic_and
#define atomic_or(mem, mask) ((void) atomic_or_val ((mem), (mask)))
#define catomic_or atomic_or
#define atomic_max(mem, val) ((void)atomic_max_val ((mem), (val)))
#define catomic_max atomic_max
#define atomic_min(mem, val) ((void)atomic_min_val ((mem), (val)))
#define catomic_min atomic_min
/*
 * This non-existent symbol is called for unsupporrted sizes,
 * indicating a bug in the caller.
 */
extern int __atomic_error_bad_argument_size(void)
    __attribute__ ((error ("bad sizeof atomic argument")));

#endif
