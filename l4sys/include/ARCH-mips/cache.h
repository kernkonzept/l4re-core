/*
 * Copyright (C) 2014 Imagination Technologies Ltd.
 * Author: Yann Le Du <ledu@kymasys.com>
 *
 * This file incorporates work covered by the following copyright notice:
 */

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
#ifndef __L4SYS__INCLUDE__ARCH_MIPS__CACHE_H__
#define __L4SYS__INCLUDE__ARCH_MIPS__CACHE_H__

#include_next <l4/sys/cache.h>
#include <l4/sys/ipc.h>
#include <l4/sys/consts.h>

EXTERN_C void syncICache(unsigned long start, unsigned long size);

L4_INLINE int
l4_cache_clean_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  l4_utcb_t *u = l4_utcb();
  l4_msg_regs_t *mr = l4_utcb_mr_u(u);
  mr->mr[0] = 0x21;
  mr->mr[1] = start;
  mr->mr[2] = end;
  return l4_error_u(l4_ipc_call(L4_INVALID_CAP, u,
                                l4_msgtag(L4_PROTO_THREAD, 3, 0, 0),
                                L4_IPC_NEVER), u);
}

L4_INLINE int
l4_cache_flush_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  l4_utcb_t *u = l4_utcb();
  l4_msg_regs_t *mr = l4_utcb_mr_u(u);
  mr->mr[0] = 0x20;
  mr->mr[1] = start;
  mr->mr[2] = end;
  return l4_error_u(l4_ipc_call(L4_INVALID_CAP, u,
                                l4_msgtag(L4_PROTO_THREAD, 3, 0, 0),
                                L4_IPC_NEVER), u);
}

L4_INLINE int
l4_cache_inv_data(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  l4_utcb_t *u = l4_utcb();
  l4_msg_regs_t *mr = l4_utcb_mr_u(u);
  mr->mr[0] = 0x22;
  mr->mr[1] = start;
  mr->mr[2] = end;
  return l4_error_u(l4_ipc_call(L4_INVALID_CAP, u,
                                l4_msgtag(L4_PROTO_THREAD, 3, 0, 0),
                                L4_IPC_NEVER), u);
}

L4_INLINE int
l4_cache_coherent(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  unsigned long step;
  unsigned long i;
  asm volatile ("rdhwr %0, $1" : "=r"(step));
  for (i = start; i < end; i += step)
    asm volatile ("synci (%0)" : : "r"(i));

  asm volatile ("sync");
  asm volatile (".set push; .set noat; .set noreorder\n"
#if (_MIPS_SZLONG == 64)
                "dla $1, 1f\n"
#else
                "la $1, 1f\n"
#endif
                "jr.hb $1\n"
                "  nop\n"
                "1: .set pop");
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

#endif /* ! __L4SYS__INCLUDE__ARCH_MIPS__CACHE_H__ */
