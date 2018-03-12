/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
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

/* Thread cancellation */

#include <errno.h>
#include <libc-internal.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"

#include <l4/sys/kdebug.h>

#ifdef _STACK_GROWS_DOWN
# define FRAME_LEFT(frame, other) ((char *) frame >= (char *) other)
#elif _STACK_GROWS_UP
# define FRAME_LEFT(frame, other) ((char *) frame <= (char *) other)
#else
# error "Define either _STACK_GROWS_DOWN or _STACK_GROWS_UP"
#endif


int
attribute_hidden
__pthread_setcancelstate(int state, int * oldstate)
{
  pthread_descr self = thread_self();
  if (state < PTHREAD_CANCEL_ENABLE || state > PTHREAD_CANCEL_DISABLE)
    return EINVAL;
  if (oldstate != NULL) *oldstate = THREAD_GETMEM(self, p_cancelstate);
  THREAD_SETMEM(self, p_cancelstate, state);
  if (THREAD_GETMEM(self, p_canceled) &&
      THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE &&
      THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
    __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
  return 0;
}
strong_alias (__pthread_setcancelstate, pthread_setcancelstate)

int
attribute_hidden
__pthread_setcanceltype(int type, int * oldtype)
{
  pthread_descr self = thread_self();
  if (type < PTHREAD_CANCEL_DEFERRED || type > PTHREAD_CANCEL_ASYNCHRONOUS)
    return EINVAL;
  if (oldtype != NULL) *oldtype = THREAD_GETMEM(self, p_canceltype);
  THREAD_SETMEM(self, p_canceltype, type);
  if (THREAD_GETMEM(self, p_canceled) &&
      THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE &&
      THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
    __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
  return 0;
}
strong_alias (__pthread_setcanceltype, pthread_setcanceltype)


/* The next two functions are similar to pthread_setcanceltype() but
   more specialized for the use in the cancelable functions like write().
   They do not need to check parameters etc.  */
int
attribute_hidden
__pthread_enable_asynccancel (void)
{
  pthread_descr self = thread_self();
  int oldtype = THREAD_GETMEM(self, p_canceltype);
  THREAD_SETMEM(self, p_canceltype, PTHREAD_CANCEL_ASYNCHRONOUS);
  if (__builtin_expect (THREAD_GETMEM(self, p_canceled), 0) &&
      THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE)
    __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
  return oldtype;
}

void
internal_function attribute_hidden
__pthread_disable_asynccancel (int oldtype)
{
  pthread_descr self = thread_self();
  THREAD_SETMEM(self, p_canceltype, oldtype);
}


#ifdef ARCH_mips
void pthread_handle_sigcancel(void);
void _pthread_handle_sigcancel(void);
void __attribute__((visibility("hidden"))) _pthread_handle_sigcancel(void)
#else
static void pthread_handle_sigcancel(void)
#endif
{
  pthread_descr self = check_thread_self();
  sigjmp_buf * jmpbuf;

  if (self == __manager_thread)
    {
      enter_kdebug("cancel of the manager hooooo");
#if 0
      __pthread_manager_sighandler(sig);
#endif
      return;
    }
  if (__builtin_expect (__pthread_exit_requested, 0)) {
#ifdef NOT_FOR_L4
    /* Main thread should accumulate times for thread manager and its
       children, so that timings for main thread account for all threads. */
    if (self == __pthread_main_thread) {
#ifdef USE_TLS
      waitpid(manager_thread->p_pid, NULL, __WCLONE);
#else
      waitpid(__pthread_manager_thread.p_pid, NULL, __WCLONE);
#endif
    }
#endif
    _exit(__pthread_exit_code);
  }
  if (__builtin_expect (THREAD_GETMEM(self, p_canceled), 0)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
    if (THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
      __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
    jmpbuf = THREAD_GETMEM(self, p_cancel_jmp);
    if (jmpbuf != NULL) {
      THREAD_SETMEM(self, p_cancel_jmp, NULL);
      siglongjmp(*jmpbuf, 1);
    }
  }
}


int pthread_cancel(pthread_t thread)
{
  pthread_handle handle = thread_handle(thread);
  int dorestart = 0;
  pthread_descr th;
  pthread_extricate_if *pextricate;
  int already_canceled;

  __pthread_lock(handle_to_lock(handle), NULL);
  if (invalid_handle(handle, thread)) {
    __pthread_unlock(handle_to_lock(handle));
    return ESRCH;
  }

  th = handle_to_descr(handle);

  already_canceled = th->p_canceled;
  th->p_canceled = 1;

  if (th->p_cancelstate == PTHREAD_CANCEL_DISABLE || already_canceled) {
    __pthread_unlock(handle_to_lock(handle));
    return 0;
  }

  pextricate = th->p_extricate;

  /* If the thread has registered an extrication interface, then
     invoke the interface. If it returns 1, then we succeeded in
     dequeuing the thread from whatever waiting object it was enqueued
     with. In that case, it is our responsibility to wake it up.
     And also to set the p_woken_by_cancel flag so the woken thread
     can tell that it was woken by cancellation. */

  if (pextricate != NULL) {
    dorestart = pextricate->pu_extricate_func(pextricate->pu_object, th);
    th->p_woken_by_cancel = dorestart;
  }

  __pthread_unlock(handle_to_lock(handle));

  /* If the thread has suspended or is about to, then we unblock it by
     issuing a restart, instead of a cancel signal. Otherwise we send
     the cancel signal to unblock the thread from a cancellation point,
     or to initiate asynchronous cancellation. The restart is needed so
     we have proper accounting of restarts; suspend decrements the thread's
     resume count, and restart() increments it.  This also means that suspend's
     handling of the cancel signal is obsolete. */

  if (dorestart)
    restart(th);
  else if (th->p_canceltype == PTHREAD_CANCEL_ASYNCHRONOUS)
    {
      l4_thread_ex_regs(th->p_th_cap, (l4_umword_t)&pthread_handle_sigcancel,
                        ~0UL, L4_THREAD_EX_REGS_CANCEL);
    }

  return 0;
}

void pthread_testcancel(void)
{
  pthread_descr self = thread_self();
  if (THREAD_GETMEM(self, p_canceled)
      && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE)
    __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
}

void
attribute_hidden
__pthread_cleanup_push(struct _pthread_cleanup_buffer * buffer,
			   void (*routine)(void *), void * arg)
{
  pthread_descr self = thread_self();
  buffer->__routine = routine;
  buffer->__arg = arg;
  buffer->__prev = THREAD_GETMEM(self, p_cleanup);
  if (buffer->__prev != NULL && FRAME_LEFT (buffer, buffer->__prev))
    buffer->__prev = NULL;
  THREAD_SETMEM(self, p_cleanup, buffer);
}

strong_alias(__pthread_cleanup_push, _pthread_cleanup_push)

void
attribute_hidden
__pthread_cleanup_pop(struct _pthread_cleanup_buffer * buffer,
			  int execute)
{
  pthread_descr self = thread_self();
  if (execute) buffer->__routine(buffer->__arg);
  THREAD_SETMEM(self, p_cleanup, buffer->__prev);
}

strong_alias(__pthread_cleanup_pop, _pthread_cleanup_pop)

void
attribute_hidden
__pthread_cleanup_push_defer(struct _pthread_cleanup_buffer * buffer,
				 void (*routine)(void *), void * arg)
{
  pthread_descr self = thread_self();
  buffer->__routine = routine;
  buffer->__arg = arg;
  buffer->__canceltype = THREAD_GETMEM(self, p_canceltype);
  buffer->__prev = THREAD_GETMEM(self, p_cleanup);
  if (buffer->__prev != NULL && FRAME_LEFT (buffer, buffer->__prev))
    buffer->__prev = NULL;
  THREAD_SETMEM(self, p_canceltype, PTHREAD_CANCEL_DEFERRED);
  THREAD_SETMEM(self, p_cleanup, buffer);
}

strong_alias(__pthread_cleanup_push_defer, _pthread_cleanup_push_defer)

void
attribute_hidden
__pthread_cleanup_pop_restore(struct _pthread_cleanup_buffer * buffer,
				  int execute)
{
  pthread_descr self = thread_self();
  if (execute) buffer->__routine(buffer->__arg);
  THREAD_SETMEM(self, p_cleanup, buffer->__prev);
  THREAD_SETMEM(self, p_canceltype, buffer->__canceltype);
  if (THREAD_GETMEM(self, p_canceled) &&
      THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE &&
      THREAD_GETMEM(self, p_canceltype) == PTHREAD_CANCEL_ASYNCHRONOUS)
    __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
}

strong_alias(__pthread_cleanup_pop_restore, _pthread_cleanup_pop_restore)

extern void __rpc_thread_destroy(void);
void
attribute_hidden internal_function
__pthread_perform_cleanup(char *currentframe)
{
  pthread_descr self = thread_self();
  struct _pthread_cleanup_buffer *c = THREAD_GETMEM(self, p_cleanup);
  struct _pthread_cleanup_buffer *last;

  if (c != NULL)
    while (FRAME_LEFT (currentframe, c))
      {
	last = c;
	c = c->__prev;

	if (c == NULL || FRAME_LEFT (last, c))
	  {
	    c = NULL;
	    break;
	  }
      }

  while (c != NULL)
    {
      c->__routine(c->__arg);

      last = c;
      c = c->__prev;

      if (FRAME_LEFT (last, c))
	break;
    }

#ifdef __UCLIBC_HAS_RPC__
  /* And the TSD which needs special help.  */
  if (THREAD_GETMEM(self, p_libc_specific[_LIBC_TSD_KEY_RPC_VARS]) != NULL)
      __rpc_thread_destroy ();
#endif
}
