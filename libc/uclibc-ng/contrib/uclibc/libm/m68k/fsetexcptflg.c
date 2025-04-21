/* Set floating-point environment exception handling.
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
#include <math.h>

int
fesetexceptflag (const fexcept_t *flagp, int excepts)
{
  fexcept_t fpsr;

  /* Get the current status register.  */
  __asm__ ("fmove%.l %/fpsr,%0" : "=dm" (fpsr));

  /* Install the new exception bits in the Accrued Exception Byte.  */
  fpsr &= ~(excepts & FE_ALL_EXCEPT);
  fpsr |= *flagp & excepts & FE_ALL_EXCEPT;

  /* Store the new status register.  */
  __asm__ __volatile__ ("fmove%.l %0,%/fpsr" : : "dm" (fpsr));

  /* Success.  */
  return 0;
}
