/* Set current rounding direction.
   Copyright (C) 2020-2025 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include "fenv_private.h"

int
fesetround (int round)
{
  unsigned int fpcr;

  _FPU_GETCW (fpcr);

  if (((fpcr >> __FPU_RND_SHIFT) & __FPU_RND_MASK) != round)
    {
      fpcr &= ~(__FPU_RND_MASK << __FPU_RND_SHIFT);
      fpcr |= (round & __FPU_RND_MASK) << __FPU_RND_SHIFT;
      _FPU_SETCW (fpcr);
    }

  return 0;
}
