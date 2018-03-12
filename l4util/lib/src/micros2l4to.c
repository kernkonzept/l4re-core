/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Michael Hohmuth <hohmuth@os.inf.tu-dresden.de>,
 *               Jork Löser <jork@os.inf.tu-dresden.de>,
 *               Lars Reuther <reuther@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/* 
 */

/*****************************************************************************
 * libl4util/src/micros2l4to.c                                               *
 * calculate L4 timeout                                                      *
 *****************************************************************************/

#include <stdio.h>
#include <l4/sys/types.h>
#include <l4/sys/kdebug.h>
#include <l4/util/util.h>
#include <l4/util/bitops.h>

L4_CV l4_timeout_s
l4util_micros2l4to(unsigned int mus)
{
  l4_timeout_s t;
  if (mus == 0)
    t = L4_IPC_TIMEOUT_0;
  else if (mus == ~0U)
    t = L4_IPC_TIMEOUT_NEVER;
  else
    {
      int e = l4util_log2(mus) - 7;
      unsigned m;

      if (e < 0) e = 0;
      m = mus / (1UL << e);

      if ((e > 31) || (m > 1023))
        {
	  printf("l4util_micros2l4to(): "
	         "invalid timeout %d, using max. values\n", mus);
	  e = 0;
	  m = 1023;
        }
      t.t = (e << 10) | m;
    }
  return t;
}

