/*!
 * \file   l4sys/lib/src/utcb.c
 * \brief  utcb
 *
 * \date   2007-11-20
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2007-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <l4/sys/utcb.h>

L4_CV __attribute__((weak)) l4_utcb_t *l4_utcb_wrap(void) L4_NOTHROW
{
  return l4_utcb_direct();
}
