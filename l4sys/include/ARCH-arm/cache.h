/**
 * \file
 * \brief  Cache functions
 *
 * \date   2007-11
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2007-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef __L4SYS__INCLUDE__ARCH_ARM__CACHE_H__
#define __L4SYS__INCLUDE__ARCH_ARM__CACHE_H__

#include <l4/sys/compiler.h>
#include <l4/sys/syscall_defs.h>

#include_next <l4/sys/cache.h>

/**
 * \internal
 */
L4_INLINE void
l4_cache_op_arm_call(unsigned long op,
                     unsigned long start,
                     unsigned long end);

L4_INLINE void
l4_cache_op_arm_call(unsigned long op,
                     unsigned long start,
                     unsigned long end)
{
  register unsigned long _op    __asm__ ("r0") = op;
  register unsigned long _start __asm__ ("r1") = start;
  register unsigned long _end   __asm__ ("r2") = end;

  __asm__ __volatile__
    ("@ l4_cache_op_arm_call(start) \n\t"
     "mov     r5, %[sc]             \n\t"
     "blx     __l4_sys_syscall      \n\t"
     "@ l4_cache_op_arm_call(end)   \n\t"
       :
	"=r" (_op),
	"=r" (_start),
	"=r" (_end)
       :
       [sc] "i" (L4_SYSCALL_MEM_OP),
	"0" (_op),
	"1" (_start),
	"2" (_end)
       :
	"cc", "memory", "r5", "ip", "lr"
       );
}

enum L4_mem_cache_ops
{
  L4_MEM_CACHE_OP_CLEAN_DATA        = 0,
  L4_MEM_CACHE_OP_FLUSH_DATA        = 1,
  L4_MEM_CACHE_OP_INV_DATA          = 2,
  L4_MEM_CACHE_OP_COHERENT          = 3,
  L4_MEM_CACHE_OP_DMA_COHERENT      = 4,
  L4_MEM_CACHE_OP_DMA_COHERENT_FULL = 5,
};

L4_INLINE int
l4_cache_clean_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  l4_cache_op_arm_call(L4_MEM_CACHE_OP_CLEAN_DATA, start, end);
  return 0;
}

L4_INLINE int
l4_cache_flush_data(unsigned long start,
                    unsigned long end) L4_NOTHROW
{
  l4_cache_op_arm_call(L4_MEM_CACHE_OP_FLUSH_DATA, start, end);
  return 0;
}

L4_INLINE int
l4_cache_inv_data(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  l4_cache_op_arm_call(L4_MEM_CACHE_OP_INV_DATA, start, end);
  return 0;
}

L4_INLINE int
l4_cache_coherent(unsigned long start,
                  unsigned long end) L4_NOTHROW
{
  l4_cache_op_arm_call(L4_MEM_CACHE_OP_COHERENT, start, end);
  return 0;
}

L4_INLINE int
l4_cache_dma_coherent(unsigned long start,
                      unsigned long end) L4_NOTHROW
{
  l4_cache_op_arm_call(L4_MEM_CACHE_OP_DMA_COHERENT, start, end);
  return 0;
}

L4_INLINE int
l4_cache_dma_coherent_full(void) L4_NOTHROW
{
  l4_cache_op_arm_call(L4_MEM_CACHE_OP_DMA_COHERENT_FULL, 0, 0);
  return 0;
}

#endif /* ! __L4SYS__INCLUDE__ARCH_ARM__CACHE_H__ */
