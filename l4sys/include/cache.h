/**
 * \file
 * Cache-consistency functions.
 *
 * \date   2007-11
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 * \ingroup l4_api
 */
/*
 * (c) 2007-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#ifndef __L4SYS__INCLUDE__CACHE_H__
#define __L4SYS__INCLUDE__CACHE_H__

#include <l4/sys/compiler.h>

/**
 * \defgroup l4_cache_api Cache Consistency
 * \ingroup l4_api
 * Various functions for cache consistency.
 *
 * These functions shall be used to ensure that
 * - all blocks (e.g. CPU cores, devices, DMA engines) are guaranteed to see
 *   the same copy of a memory location (Point of Coherency -- PoC),
 * - instruction and data caches of a core are guaranteed to see the same copy
 *   of a memory location (Point of Unification -- PoU).
 *
 * Certain functions are NOPs on certain architectures, for example on Intel
 * it's not necessary to explicitly make caches coherent to PoU.
 */

EXTERN_C_BEGIN

/**
 * Cache clean a range in D-cache; writes back to PoC.
 * \ingroup l4_cache_api
 *
 * \param start  Start of range (inclusive)
 * \param end    End of range (exclusive)
 *
 * \retval 0        on success
 * \retval -EFAULT  in the case of an unresolved page fault
 *                  in the given area
 *
 * Writes back any dirty cache lines in the range but leaves them in the cache
 * and marks the cached copies clean.
 */
L4_INLINE int
l4_cache_clean_data(unsigned long start,
                    unsigned long end) L4_NOTHROW;

/**
 * Cache flush a range; writes back to PoC.
 * \ingroup l4_cache_api
 *
 * \param start  Start of range (inclusive)
 * \param end    End of range (exclusive)
 *
 * \retval 0        on success
 * \retval -EFAULT  in the case of an unresolved page fault
 *                  in the given area
 *
 * Writes back any dirty cache lines and invalidates all cache entries in the
 * range.
 */
L4_INLINE int
l4_cache_flush_data(unsigned long start,
                    unsigned long end) L4_NOTHROW;

/**
 * Cache invalidate a range; might write back to PoC.
 * \ingroup l4_cache_api
 *
 * \param start  Start of range (inclusive)
 * \param end    End of range (exclusive)
 *
 * \retval 0        on success
 * \retval -EFAULT  in the case of an unresolved page fault
 *                  in the given area
 *
 * Invalidates all cache entries in the range but does not necessarily write
 * back dirty cache lines.
 *
 * \note Implementations may choose to write back dirty
 *       lines nonetheless if this is more efficient.
 *
 */
L4_INLINE int
l4_cache_inv_data(unsigned long start,
                  unsigned long end) L4_NOTHROW;

/**
 * Make memory coherent between I-cache and D-cache; writes back to PoU.
 * \ingroup l4_cache_api
 *
 * \param start  Start of range (inclusive)
 * \param end    End of range (exclusive)
 *
 * \retval 0        on success
 * \retval -EFAULT  in the case of an unresolved page fault
 *                  in the given area
 */
L4_INLINE int
l4_cache_coherent(unsigned long start,
                  unsigned long end) L4_NOTHROW;

/**
 * Make memory coherent for use with external memory; writes back to PoC.
 * \ingroup l4_cache_api
 *
 * \param start  Start of range (inclusive)
 * \param end    End of range (exclusive)
 *
 * \retval 0        on success
 * \retval -EFAULT  in the case of an unresolved page fault
 *                  in the given area
 */
L4_INLINE int
l4_cache_dma_coherent(unsigned long start,
                      unsigned long end) L4_NOTHROW;

#if !defined(ARCH_arm64)
/**
 * Make memory coherent for use with external memory; writes back to PoC.
 * \ingroup l4_cache_api
 */
L4_INLINE int
l4_cache_dma_coherent_full(void) L4_NOTHROW;
#endif

EXTERN_C_END

#endif /* ! __L4SYS__INCLUDE__CACHE_H__ */
