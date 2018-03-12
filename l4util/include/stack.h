/**
 * \file
 * \brief  Some helper functions for stack manipulation. Newer versions of
 *         gcc forbid to cast the lvalue of an expression resulting that
 *         the following expression is invalid:
 *
 *         *--((l4_threadid_t)esp) = tid
 *
 * \date   03/2004
 * \author Frank Mehnert <fm3@os.inf.tu-dresden.de> */

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef _L4UTIL_STACK_H
#define _L4UTIL_STACK_H

#include <l4/sys/types.h>
#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

L4_INLINE void l4util_stack_push_mword(l4_addr_t *stack, l4_mword_t val);

/*****************************************************************************/
/**
 * \brief Get current stack pointer.
 *
 * \return stack pointer.
 */
L4_INLINE l4_addr_t l4util_stack_get_sp(void);

/*
 * Implementations.
 */

#include <l4/util/stack_impl.h>

L4_INLINE void
l4util_stack_push_mword(l4_addr_t *stack, l4_mword_t val)
{
  l4_mword_t *esp = (l4_mword_t*)(*stack);
  *--esp = val;
  *stack = (l4_addr_t)esp;
}

EXTERN_C_END

#endif
