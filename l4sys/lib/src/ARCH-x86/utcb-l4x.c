/*!
 * \file
 * \brief  utcb for L4Linux programs
 *
 * \date   2008-02-25
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2008-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/sys/utcb.h>

L4_CV l4_utcb_t *l4_utcb_wrap(void) L4_NOTHROW
{
  l4_utcb_t *utcb;
  __asm__ __volatile__ ("mov %%fs:0, %0" : "=r" (utcb));
  return utcb;
}
