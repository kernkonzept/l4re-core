/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stddef.h>

void *memset(void *s, int c, size_t n);

void *memset(void *s, int c, size_t n)
{
  size_t x;
  char *p = s;
  for (x=0; x<n; ++x)
    *p++ = c;

  return s;
}
