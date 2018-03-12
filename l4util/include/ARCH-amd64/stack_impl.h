/**
 * \file
 * \brief Stack utilities for amd64
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __L4UTIL__INCLUDE__ARCH_AMD64__STACK_IMPL_H__
#define __L4UTIL__INCLUDE__ARCH_AMD64__STACK_IMPL_H__

EXTERN_C_BEGIN

#ifndef _L4UTIL_STACK_H
#error Do not include stack_impl.h directly, use stack.h instead
#endif

L4_INLINE l4_addr_t l4util_stack_get_sp(void)
{
  l4_addr_t rsp;

  asm("movq   %%rsp, %0\n\t" : "=r" (rsp) : );
  return rsp;
}

EXTERN_C_END

#endif /* ! __L4UTIL__INCLUDE__ARCH_AMD64__STACK_IMPL_H__ */
