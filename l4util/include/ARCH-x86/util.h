/**
 * \file
 * \brief Utilities for x86
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Jork Löser <jork@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __UTIL_H
#define __UTIL_H

#include <l4/sys/types.h>
#include <l4/sys/compiler.h>
#include <l4/sys/ipc.h>

EXTERN_C_BEGIN

/** Calculate l4 timeouts
 * \param mus	time in microseconds. Special cases:
 *		- 0 - > timeout 0
 *		- ~0U -> timeout NEVER
 * \return the corresponding l4_timeout value
 */
L4_CV l4_timeout_s l4util_micros2l4to(unsigned int mus) L4_NOTHROW;

/** Suspend thread for a period of \a ms milliseconds
 * \param ms Time in milliseconds
 */
L4_CV void l4_sleep(int ms) L4_NOTHROW;

/* Suspend thread for a period of \a us microseconds.
 * \param us Time in microseconds
 * WARNING: This function is mostly bogus since the timer resolution of
 *          current L4 implementations is about 1ms! */
L4_CV void l4_usleep(int us) L4_NOTHROW;

/**
 * \brief Go sleep and never wake up.
 * \ingroup l4util_api
 *
 */
L4_INLINE void l4_sleep_forever(void) L4_NOTHROW __attribute__((noreturn));

L4_INLINE void
l4_sleep_forever(void) L4_NOTHROW
{
  for (;;)
    l4_ipc_sleep(L4_IPC_NEVER);
}

/** Touch data areas to force mapping read-only */
static inline void
l4_touch_ro(const void*addr, unsigned size) L4_NOTHROW
{
  const char *bptr, *eptr;

  bptr = (const char*)(((l4_addr_t)addr) & L4_PAGEMASK);
  eptr = (const char*)(((l4_addr_t)addr+size-1) & L4_PAGEMASK);
  for(;bptr<=eptr;bptr+=L4_PAGESIZE){
    asm volatile("or	%0,%%eax \n"
                 :
                 : "m" (*(const unsigned*)bptr)
                 : "eax" );
  }
}


/** Touch data areas to force mapping read-write */
static inline void
l4_touch_rw(const void*addr, unsigned size) L4_NOTHROW
{
  const char *bptr, *eptr;

  bptr = (const char*)(((l4_addr_t)addr) & L4_PAGEMASK);
  eptr = (const char*)(((l4_addr_t)addr+size-1) & L4_PAGEMASK);
  for(;bptr<=eptr;bptr+=L4_PAGESIZE){
    asm volatile("orb	$0,%0 \n"
                 :
                 : "m" (*(const unsigned*)bptr)
                 );
  }
}

EXTERN_C_END

#endif

