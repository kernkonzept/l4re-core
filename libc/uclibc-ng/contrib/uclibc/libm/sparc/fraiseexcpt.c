/* Raise given exceptions.
   Copyright (C) 1997-2025 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include <float.h>
#include <math.h>
#include "math-barriers.h"

int
feraiseexcept (int excepts)
{
  static const struct {
    double zero, one, max, min, pi;
  } c = {
    0.0, 1.0, DBL_MAX, DBL_MIN, M_PI
  };
  double d;

  /* Raise exceptions represented by EXPECTS.  But we must raise only
     one signal at a time.  It is important the if the overflow/underflow
     exception and the inexact exception are given at the same time,
     the overflow/underflow exception follows the inexact exception.  */

  /* First: invalid exception.  */
  if ((FE_INVALID & excepts) != 0)
    {
      /* One example of an invalid operation is 0/0.  */
      __asm__ ("" : "=e" (d) : "0" (c.zero));
      d /= c.zero;
      math_force_eval (d);
    }

  /* Next: division by zero.  */
  if ((FE_DIVBYZERO & excepts) != 0)
    {
      __asm__ ("" : "=e" (d) : "0" (c.one));
      d /= c.zero;
      math_force_eval (d);
    }

  /* Next: overflow.  */
  if ((FE_OVERFLOW & excepts) != 0)
    {
      __asm__ ("" : "=e" (d) : "0" (c.max));
      d *= d;
      math_force_eval (d);
    }

  /* Next: underflow.  */
  if ((FE_UNDERFLOW & excepts) != 0)
    {
      __asm__ ("" : "=e" (d) : "0" (c.min));
      d *= d;
      math_force_eval (d);
    }

  /* Last: inexact.  */
  if ((FE_INEXACT & excepts) != 0)
    {
      __asm__ ("" : "=e" (d) : "0" (c.one));
      d /= c.pi;
      math_force_eval (d);
    }

  /* Success.  */
  return 0;
}
