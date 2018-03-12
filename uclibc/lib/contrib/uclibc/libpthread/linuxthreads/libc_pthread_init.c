/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#ifdef __UCLIBC_HAS_TLS__
#include <tls.h>
#endif
#include "internals.h"


int __libc_multiple_threads attribute_hidden __attribute__((nocommon));
strong_alias (__libc_multiple_threads, __librt_multiple_threads)


int *
__libc_pthread_init(const struct pthread_functions *functions)
{
#ifdef SHARED
  /* We copy the content of the variable pointed to by the FUNCTIONS
     parameter to one in libc.so since this means access to the array
     can be done with one memory access instead of two.  */
  memcpy (&__libc_pthread_functions, functions,
	  sizeof (__libc_pthread_functions));
#endif

#ifndef __UCLIBC_HAS_TLS__
  /* Initialize thread-locale current locale to point to the global one.
     With __thread support, the variable's initializer takes care of this.  */
  __uselocale (LC_GLOBAL_LOCALE);
#endif

  return &__libc_multiple_threads;
}
