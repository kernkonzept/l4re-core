/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Michael Hohmuth <hohmuth@os.inf.tu-dresden.de>,
 *               Jork Löser <jork@os.inf.tu-dresden.de>,
 *               Lars Reuther <reuther@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/* 
 */

/*****************************************************************************
 * libl4util/src/micros2l4to.c                                               *
 * calculate L4 timeout                                                      *
 *****************************************************************************/

#include <l4/sys/types.h>
#include <l4/util/util.h>

L4_CV l4_timeout_s
l4util_micros2l4to(l4_uint64_t us)
{
  return l4_timeout_from_us(us);
}
