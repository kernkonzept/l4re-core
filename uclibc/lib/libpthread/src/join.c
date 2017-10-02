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

/* Thread termination and joining */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"

#include <l4/util/util.h>

void
attribute_hidden
__pthread_exit(void * retval)
{
  __pthread_do_exit (retval, CURRENT_STACK_FRAME);
}
strong_alias (__pthread_exit, pthread_exit)

void
attribute_hidden
__pthread_do_exit(void *retval, char *currentframe)
{
  pthread_descr self = thread_self();
  pthread_descr joining;
  struct pthread_request request;

  /* Reset the cancellation flag to avoid looping if the cleanup handlers
     contain cancellation points */
  THREAD_SETMEM(self, p_canceled, 0);
  /* Call cleanup functions and destroy the thread-specific data */
  __pthread_perform_cleanup(currentframe);
  __pthread_destroy_specifics();
  /* Store return value */
  __pthread_lock(THREAD_GETMEM(self, p_lock), self);
  THREAD_SETMEM(self, p_retval, retval);
  /* Say that we've terminated */
  THREAD_SETMEM(self, p_terminated, 1);
  /* See if someone is joining on us */
  joining = THREAD_GETMEM(self, p_joining);
  __pthread_unlock(THREAD_GETMEM(self, p_lock));
  /* Restart joining thread if any */
  if (joining != NULL)
    {
      restart(joining);
      l4_sleep_forever();
    }

  /* If this is the initial thread, block until all threads have terminated.
     If another thread calls exit, we'll be terminated from our signal
     handler. */
  if (self == __pthread_main_thread && __pthread_manager_request >= 0)
    {
      request.req_thread = self;
      request.req_kind = REQ_MAIN_THREAD_EXIT;
      __pthread_send_manager_rq(&request, 1);
    /* Main thread flushes stdio streams and runs atexit functions.
       It also calls a handler within LinuxThreads which sends a process exit
       request to the thread manager. */
    exit(0);
  }
  /* Threads other than the main one  terminate without flushing stdio streams
     or running atexit functions. */

  //_exit(0);

  request.req_thread = self;
  request.req_kind = REQ_THREAD_EXIT;
  __pthread_send_manager_rq(&request, 1);

  l4_sleep_forever();
}

/* Function called by pthread_cancel to remove the thread from
   waiting on a condition variable queue. */

static int join_extricate_func(void *obj, pthread_descr th)
{
  __volatile__ pthread_descr self = thread_self();
  pthread_handle handle = obj;
  pthread_descr jo;
  int did_remove = 0;

  __pthread_lock(handle_to_lock(handle), self);
  jo = handle_to_descr(handle);
  did_remove = jo->p_joining != NULL;
  jo->p_joining = NULL;
  __pthread_unlock(handle_to_lock(handle));

  return did_remove;
}

int pthread_join(pthread_t thread_id, void ** thread_return)
{
  __volatile__ pthread_descr self = thread_self();
  struct pthread_request request;
  pthread_handle handle = thread_handle(thread_id);
  pthread_descr th;
  pthread_extricate_if extr;
  int already_canceled = 0;

  /* Set up extrication interface */
  extr.pu_object = handle;
  extr.pu_extricate_func = join_extricate_func;

  __pthread_lock(handle_to_lock(handle), self);
  if (nonexisting_handle(handle, thread_id)) {
    __pthread_unlock(handle_to_lock(handle));
    return ESRCH;
  }
  th = handle_to_descr(handle);
  if (th == self) {
    __pthread_unlock(handle_to_lock(handle));
    return EDEADLK;
  }
  /* If detached or already joined, error */
  if (th->p_detached || th->p_joining != NULL) {
    __pthread_unlock(handle_to_lock(handle));
    return EINVAL;
  }
  /* If not terminated yet, suspend ourselves. */
  if (! th->p_terminated) {
    /* Register extrication interface */
    __pthread_set_own_extricate_if(self, &extr);
    if (!(THREAD_GETMEM(self, p_canceled)
	&& THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE))
      th->p_joining = self;
    else
      already_canceled = 1;
    __pthread_unlock(handle_to_lock(handle));

    if (already_canceled) {
      __pthread_set_own_extricate_if(self, 0);
      __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
    }

    suspend(self);
    /* Deregister extrication interface */
    __pthread_set_own_extricate_if(self, 0);

    /* This is a cancellation point */
    if (THREAD_GETMEM(self, p_woken_by_cancel)
	&& THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE) {
      THREAD_SETMEM(self, p_woken_by_cancel, 0);
      __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
    }
    __pthread_lock(handle_to_lock(handle), self);
  }
  /* Get return value */
  if (thread_return != NULL) *thread_return = th->p_retval;
  __pthread_unlock(handle_to_lock(handle));
  /* Send notification to thread manager */
  if (__pthread_manager_request >= 0) {
    request.req_thread = self;
    request.req_kind = REQ_FREE;
    request.req_args.free.thread_id = thread_id;
    __pthread_send_manager_rq(&request, 0);
  }
  return 0;
}

int pthread_detach(pthread_t thread_id)
{
  int terminated;
  struct pthread_request request;
  pthread_handle handle = thread_handle(thread_id);
  pthread_descr th;

  __pthread_lock(handle_to_lock(handle), NULL);
  if (nonexisting_handle(handle, thread_id)) {
    __pthread_unlock(handle_to_lock(handle));
    return ESRCH;
  }
  th = handle_to_descr(handle);
  /* If already detached, error */
  if (th->p_detached) {
    __pthread_unlock(handle_to_lock(handle));
    return EINVAL;
  }
  /* If already joining, don't do anything. */
  if (th->p_joining != NULL) {
    __pthread_unlock(handle_to_lock(handle));
    return 0;
  }
  /* Mark as detached */
  th->p_detached = 1;
  terminated = th->p_terminated;
  __pthread_unlock(handle_to_lock(handle));
  /* If already terminated, notify thread manager to reclaim resources */
  if (terminated && __pthread_manager_request >= 0) {
    request.req_thread = thread_self();
    request.req_kind = REQ_FREE;
    request.req_args.free.thread_id = thread_id;
    __pthread_send_manager_rq(&request, 0);
  }
  return 0;
}
