/* Store current floating-point environment.
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

int
fegetenv (fenv_t *envp)
{
#ifdef __mcoldfire__
  __asm__ ("fmove%.l %/fpcr,%0" : "=dm" (envp->__control_register));
  __asm__ ("fmove%.l %/fpsr,%0" : "=dm" (envp->__status_register));
  __asm__ ("fmove%.l %/fpiar,%0" : "=dm" (envp->__instruction_address));
#else
  __asm__ ("fmovem%.l %/fpcr/%/fpsr/%/fpiar,%0" : "=m" (*envp));
#endif

  /* Success.  */
  return 0;
}
