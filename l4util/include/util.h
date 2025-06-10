/**
 * Utilities, generic file
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef __L4UTIL__UTIL_H__
#define __L4UTIL__UTIL_H__

#include <l4/sys/types.h>
#include <l4/sys/compiler.h>
#include <l4/sys/ipc.h>

/**
 * \defgroup l4util_api Utility Functions
 */

L4_BEGIN_DECLS

/**
 * Calculate l4 timeouts
 * \ingroup l4util_api
 * \param us    time in microseconds. Special cases:
 *              - 0 - > timeout 0
 *              - ~0U -> timeout NEVER
 * \return the corresponding l4_timeout value
 *
 * \deprecated Use l4_timeout_from_us().
 */
L4_CV l4_timeout_s l4util_micros2l4to(l4_uint64_t us) L4_NOTHROW;

/**
 * Suspend thread for a period of \a ms milliseconds
 * \param ms Time in milliseconds
 * \ingroup l4util_api
 */
L4_CV void l4_sleep(l4_uint32_t ms) L4_NOTHROW;

/**
 * Suspend thread for a period of \a us microseconds.
 * \param us Time in microseconds
 * \ingroup l4util_api
 * \note The timer resolution of L4 kernels is usually 1ms.
 */
L4_CV void l4_usleep(l4_uint64_t us) L4_NOTHROW;

/**
 * Go sleep and never wake up.
 * \ingroup l4util_api
 *
 */
L4_INLINE void l4_sleep_forever(void) L4_NOTHROW L4_NORETURN;

/**
 * Touch data area to force mapping (read-only)
 * \ingroup l4util_api
 *
 * \param addr Start of memory area to touch.
 * \param size Size of area to touch.
 */
L4_INLINE void
l4_touch_ro(const void *addr, unsigned size) L4_NOTHROW;

/**
 * Touch data areas to force mapping (read-write)
 * \ingroup l4util_api
 *
 * \param addr Start of memory area to touch.
 * \param size Size of area to touch.
 */
L4_INLINE void
l4_touch_rw(const void *addr, unsigned size) L4_NOTHROW;



/*
 * Implementations
 */

L4_INLINE void
l4_sleep_forever(void) L4_NOTHROW
{
  for (;;)
    l4_ipc_sleep(L4_IPC_NEVER);
}

L4_INLINE void
l4_touch_ro(const void *addr, unsigned size) L4_NOTHROW
{
  l4_addr_t b, e;

  b = l4_trunc_page((l4_addr_t)addr);
  e = l4_trunc_page((l4_addr_t)addr + size - 1);

  for (; b <= e; b += L4_PAGESIZE)
    (void)(*(volatile char *)b);
}


L4_INLINE void
l4_touch_rw(const void *addr, unsigned size) L4_NOTHROW
{
  l4_addr_t b, e;

  b = l4_trunc_page((l4_addr_t)addr);
  e = l4_trunc_page((l4_addr_t)addr + size - 1);

  for (; b <= e; b += L4_PAGESIZE)
    *(volatile char *)b |= 0;
}

L4_END_DECLS

#endif /* __L4UTIL__UTIL_H__ */
