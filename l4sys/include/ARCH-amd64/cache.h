/**
 * \file
 * \brief  Cache functions
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
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
#ifndef __L4SYS__INCLUDE__ARCH_AMD64__CACHE_H__
#define __L4SYS__INCLUDE__ARCH_AMD64__CACHE_H__

#include_next <l4/sys/cache.h>

L4_INLINE int
l4_cache_clean_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  (void)start; (void)end;
  return 0;
}

L4_INLINE int
l4_cache_flush_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  (void)start; (void)end;
  return 0;
}

L4_INLINE int
l4_cache_inv_data(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  (void)start; (void)end;
  return 0;
}

L4_INLINE int
l4_cache_coherent(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  (void)start; (void)end;
  return 0;
}

L4_INLINE int
l4_cache_dma_coherent(unsigned long start,
                      unsigned long end) L4_NOTHROW
{
  (void)start; (void)end;
  return 0;
}

L4_INLINE int
l4_cache_dma_coherent_full(void) L4_NOTHROW
{
  return 0;
}

#endif /* ! __L4SYS__INCLUDE__ARCH_AMD64__CACHE_H__ */
