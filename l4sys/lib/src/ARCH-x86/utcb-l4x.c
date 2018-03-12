/*!
 * \file   l4sys/lib/src/utcb.c
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

#include <l4/sys/utcb.h>

L4_CV l4_utcb_t *l4_utcb_wrap(void)
{
  l4_utcb_t *utcb;
  __asm__ __volatile__ ("mov %%fs:0, %0" : "=r" (utcb));
  return utcb;
}
