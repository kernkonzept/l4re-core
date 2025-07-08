/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1998 Xavier Leroy (Xavier.Leroy@inria.fr)	        */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Redefine siglongjmp and longjmp so that they interact correctly
   with cleanup handlers */

#include <setjmp.h>
#include "pthread.h"
#include "internals.h"
#include <jmpbuf-unwind.h>

void
attribute_hidden
__pthread_cleanup_upto (__jmp_buf target, char *targetframe)
{
  pthread_descr self = thread_self();
  struct _pthread_cleanup_buffer * c;

  for (c = THREAD_GETMEM(self, p_cleanup);
       c != NULL && _JMPBUF_UNWINDS(target, c);
       c = c->__prev)
    {
#if _STACK_GROWS_DOWN
      if ((char *) c <= targetframe)
	{
	  c = NULL;
	  break;
	}
#elif _STACK_GROWS_UP
      if ((char *) c >= targetframe)
	{
	  c = NULL;
	  break;
	}
#else
# error "Define either _STACK_GROWS_DOWN or _STACK_GROWS_UP"
#endif
      c->__routine(c->__arg);
    }
  THREAD_SETMEM(self, p_cleanup, c);
#ifdef NOT_FOR_L4
  if (THREAD_GETMEM(self, p_in_sighandler)
      && _JMPBUF_UNWINDS(target, THREAD_GETMEM(self, p_in_sighandler)))
    THREAD_SETMEM(self, p_in_sighandler, NULL);
#endif
}
