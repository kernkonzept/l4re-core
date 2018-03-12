/*****************************************************************************/
/**
 * \file
 * \brief   Simple lock implementation. 
 *          Does only work if all thread have the same priority!
 *
 * \date    02/1997
 * \author  Michael Hohmuth <hohmuth@os.inf.tu-dresden.de> */
/*
 * (c) 2000-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

/*****************************************************************************/
#ifndef __L4UTIL_LOCK_H__
#define __L4UTIL_LOCK_H__

#include <l4/sys/thread.h>
#include <l4/sys/compiler.h>
#include <l4/util/atomic.h>

EXTERN_C_BEGIN

typedef l4_uint32_t l4util_simple_lock_t;

L4_INLINE int  l4_simple_try_lock(l4util_simple_lock_t *lock);
L4_INLINE void l4_simple_unlock(l4util_simple_lock_t *lock);
L4_INLINE int  l4_simple_lock_locked(l4util_simple_lock_t *lock);
L4_INLINE void l4_simple_lock_solid(register l4util_simple_lock_t *p);
L4_INLINE void l4_simple_lock(l4util_simple_lock_t * lock);

L4_INLINE int 
l4_simple_try_lock(l4util_simple_lock_t *lock)
{
  return l4util_xchg32(lock, 1) == 0;
}
 
L4_INLINE void 
l4_simple_unlock(l4util_simple_lock_t *lock)
{
  *lock = 0;
}

L4_INLINE int
l4_simple_lock_locked(l4util_simple_lock_t *lock)
{
  return (*lock == 0) ? 0 : 1;
}

L4_INLINE void
l4_simple_lock_solid(register l4util_simple_lock_t *p)
{
  while (l4_simple_lock_locked(p) || !l4_simple_try_lock(p))
    l4_thread_switch(L4_INVALID_CAP);
}

L4_INLINE void
l4_simple_lock(l4util_simple_lock_t * lock)
{
  if (!l4_simple_try_lock(lock))
    l4_simple_lock_solid(lock);
}

EXTERN_C_END

#endif
