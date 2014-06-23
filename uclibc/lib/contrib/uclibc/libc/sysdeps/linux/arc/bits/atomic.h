/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <stdint.h>
#include <sysdep.h>

typedef int8_t atomic8_t;
typedef uint8_t uatomic8_t;
typedef int_fast8_t atomic_fast8_t;
typedef uint_fast8_t uatomic_fast8_t;

typedef int32_t atomic32_t;
typedef uint32_t uatomic32_t;
typedef int_fast32_t atomic_fast32_t;
typedef uint_fast32_t uatomic_fast32_t;

typedef intptr_t atomicptr_t;
typedef uintptr_t uatomicptr_t;
typedef intmax_t atomic_max_t;
typedef uintmax_t uatomic_max_t;

void __arc_link_error (void);

#define atomic_full_barrier() \
     __asm__ __volatile__("": : :"memory")

/* Atomic compare and exchange. */

#define __arch_compare_and_exchange_val_8_acq(mem, newval, oldval) \
  ({ __arc_link_error (); oldval; })

#define __arch_compare_and_exchange_val_16_acq(mem, newval, oldval) \
  ({ __arc_link_error (); oldval; })

#define __arch_compare_and_exchange_val_32_acq(mem, newval, oldval)     \
  ({									\
	__typeof(oldval) prev;						\
									\
	__asm__ __volatile__(						\
	"1:	llock   %0, [%1]	\n"				\
	"	brne    %0, %2, 2f	\n"				\
	"	scond   %3, [%1]	\n"				\
	"	bnz     1b		\n"				\
	"2:				\n"				\
	: "=&r"(prev)							\
	: "r"(mem), "ir"(oldval),					\
	  "r"(newval) /* can't be "ir". scond can't take limm for "b" */\
	: "cc", "memory");						\
									\
	prev;								\
  })

#define __arch_compare_and_exchange_val_64_acq(mem, newval, oldval) \
  ({ __arc_link_error (); oldval; })
