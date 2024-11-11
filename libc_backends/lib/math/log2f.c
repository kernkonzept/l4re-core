/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/* uclibc has it disabled for some unknown reason, lets put it here */

#include <math.h>

float log2f(float x)
{
  return (float)log2((double)x);
}
