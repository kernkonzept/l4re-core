/*
 * (c) 2008-2009 Frank Mehnert <fm3@os.inf.tu-dresden.de>,
 *               Michael Hohmuth <hohmuth@os.inf.tu-dresden.de>,
 *               Lars Reuther <reuther@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/*
 */

/*****************************************************************************
 * random.c                                                                  *
 * pseudo-random number generator                                            *
 *****************************************************************************/

#include <l4/util/rand.h>

static unsigned int l4_rand_next = 1;

L4_CV l4_uint32_t
l4util_rand(void)
{
  l4_rand_next = l4_rand_next * 1103515245 + 12345;
  return ((l4_rand_next >>16) & L4_RAND_MAX);
}

L4_CV void
l4util_srand (l4_uint32_t seed)
{
  l4_rand_next = seed;
}

