/* Install given floating-point environment and raise exceptions.
   Copyright (C) 2013 Imagination Technologies Ltd.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <fenv.h>
#include <unistd.h>

libm_hidden_proto(fesetenv)
libm_hidden_proto(feraiseexcept)

int
feupdateenv (const fenv_t *envp)
{
  unsigned int temp;

  /* Save current exceptions.  */
  __asm__ ("MOV %0,TXDEFR" : "=r" (temp));

  temp >>= 16;
  temp &= FE_ALL_EXCEPT;

  /* Install new environment.  */
  fesetenv (envp);

  /* Raise the saved exception.  */
  feraiseexcept ((int) temp);

  /* Success.  */
  return 0;
}
