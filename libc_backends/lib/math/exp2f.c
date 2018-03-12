/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/* uclibc has it disabled for some unknown reason, lets put it here */

#include <math.h>

float exp2f(float x)
{
  return (float)exp2((double)x);
}
