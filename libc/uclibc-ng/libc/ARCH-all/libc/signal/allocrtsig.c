/* Handle real-time signal allocation.
   Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
   <http://www.gnu.org/licenses/>.  */

#include <features.h>
#include <signal.h>
#include <sys/syscall.h>

static int current_rtmin = __SIGRTMIN;
static int current_rtmax = __SIGRTMAX;

/* Return number of available real-time signal with highest priority.  */
int __libc_current_sigrtmin (void)
{
  return current_rtmin;
}

/* Return number of available real-time signal with lowest priority.  */
int __libc_current_sigrtmax (void)
{
  return current_rtmax;
}

