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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include <float.h>
#include <math.h>

int
feraiseexcept (int excepts)
{
#if defined(__mcffpu__)

  /* The Coldfire FPU allows an exception to be raised by asserting
     the associated EXC bit and then executing an arbitrary arithmetic
     instruction.  fmove.l is classified as an arithmetic instruction
     and suffices for this purpose.

     We therefore raise an exception by setting both the EXC and AEXC
     bit associated with the exception (the former being 6 bits to the
     left of the latter) and then loading the longword at (%sp) into an
     FP register.  */

  inline void
  raise_one_exception (int mask)
  {
    if (excepts & mask)
      {
	int fpsr;
	double unused;

	__asm__ volatile ("fmove%.l %/fpsr,%0" : "=d" (fpsr));
	fpsr |= (mask << 6) | mask;
	__asm__ volatile ("fmove%.l %0,%/fpsr" :: "d" (fpsr));
	__asm__ volatile ("fmove%.l (%%sp),%0" : "=f" (unused));
      }
  }

  raise_one_exception (FE_INVALID);
  raise_one_exception (FE_DIVBYZERO);
  raise_one_exception (FE_OVERFLOW);
  raise_one_exception (FE_UNDERFLOW);
  raise_one_exception (FE_INEXACT);

#else
  /* Raise exceptions represented by EXCEPTS.  But we must raise only one
     signal at a time.  It is important that if the overflow/underflow
     exception and the divide by zero exception are given at the same
     time, the overflow/underflow exception follows the divide by zero
     exception.  */

  /* First: invalid exception.  */
  if (excepts & FE_INVALID)
    {
      /* One example of an invalid operation is 0 * Infinity.  */
      double d = HUGE_VAL;
      __asm__ __volatile__ ("fmul%.s %#0r0,%0; fnop" : "=f" (d) : "0" (d));
    }

  /* Next: division by zero.  */
  if (excepts & FE_DIVBYZERO)
    {
      double d = 1.0;
      __asm__ __volatile__ ("fdiv%.s %#0r0,%0; fnop" : "=f" (d) : "0" (d));
    }

  /* Next: overflow.  */
  if (excepts & FE_OVERFLOW)
    {
      long double d = LDBL_MAX;

      __asm__ __volatile__ ("fmul%.x %0,%0; fnop" : "=f" (d) : "0" (d));
    }

  /* Next: underflow.  */
  if (excepts & FE_UNDERFLOW)
    {
      long double d = -LDBL_MAX;

      __asm__ __volatile__ ("fetox%.x %0; fnop" : "=f" (d) : "0" (d));
    }

  /* Last: inexact.  */
  if (excepts & FE_INEXACT)
    {
      long double d = 1.0;
      __asm__ __volatile__ ("fdiv%.s %#0r3,%0; fnop" : "=f" (d) : "0" (d));
    }

#endif
  /* Success.  */
  return 0;
}
