/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#ifndef _PTHREAD_FUNCTIONS_H
#define _PTHREAD_FUNCTIONS_H	1

#include <pthread.h>
#if 0
#include <setjmp.h>
#include <linuxthreads.old/internals.h>

struct fork_block;
#endif

/* Data type shared with libc.  The libc uses it to pass on calls to
   the thread functions.  Wine pokes directly into this structure,
   so if possible avoid breaking it and append new hooks to the end.  */
struct pthread_functions
{
#if 0
  pid_t (*ptr_pthread_fork) (struct fork_block *);
#endif
  int (*ptr_pthread_attr_destroy) (pthread_attr_t *);
  int (*ptr_pthread_attr_init) (pthread_attr_t *);
  int (*ptr_pthread_attr_getdetachstate) (const pthread_attr_t *, int *);
  int (*ptr_pthread_attr_setdetachstate) (pthread_attr_t *, int);
  int (*ptr_pthread_attr_getinheritsched) (const pthread_attr_t *, int *);
  int (*ptr_pthread_attr_setinheritsched) (pthread_attr_t *, int);
  int (*ptr_pthread_attr_getschedparam) (const pthread_attr_t *,
					 struct sched_param *);
  int (*ptr_pthread_attr_setschedparam) (pthread_attr_t *,
					 const struct sched_param *);
  int (*ptr_pthread_attr_getschedpolicy) (const pthread_attr_t *, int *);
  int (*ptr_pthread_attr_setschedpolicy) (pthread_attr_t *, int);
  int (*ptr_pthread_attr_getscope) (const pthread_attr_t *, int *);
  int (*ptr_pthread_attr_setscope) (pthread_attr_t *, int);
  int (*ptr_pthread_condattr_destroy) (pthread_condattr_t *);
  int (*ptr_pthread_condattr_init) (pthread_condattr_t *);
  int (*ptr_pthread_cond_broadcast) (pthread_cond_t *);
  int (*ptr_pthread_cond_destroy) (pthread_cond_t *);
  int (*ptr_pthread_cond_init) (pthread_cond_t *,
				  const pthread_condattr_t *);
  int (*ptr_pthread_cond_signal) (pthread_cond_t *);
  int (*ptr_pthread_cond_wait) (pthread_cond_t *, pthread_mutex_t *);
  int (*ptr_pthread_equal) (pthread_t, pthread_t);
  void (*ptr___pthread_exit) (void *);
  int (*ptr_pthread_getschedparam) (pthread_t, int *, struct sched_param *);
  int (*ptr_pthread_setschedparam) (pthread_t, int,
				    const struct sched_param *);
  int (*ptr_pthread_mutex_destroy) (pthread_mutex_t *);
  int (*ptr_pthread_mutex_init) (pthread_mutex_t *,
				 const pthread_mutexattr_t *);
  int (*ptr_pthread_mutex_lock) (pthread_mutex_t *);
  int (*ptr_pthread_mutex_trylock) (pthread_mutex_t *);
  int (*ptr_pthread_mutex_unlock) (pthread_mutex_t *);
  pthread_t (*ptr_pthread_self) (void);
  int (*ptr_pthread_setcancelstate) (int, int *);
  int (*ptr_pthread_setcanceltype) (int, int *);
#if 0
  void (*ptr_pthread_do_exit) (void *retval, char *currentframe);
  void (*ptr_pthread_cleanup_upto) (__jmp_buf target,
				    char *targetframe);
  pthread_descr (*ptr_pthread_thread_self) (void);
#endif
#if !defined __UCLIBC_HAS_TLS__ && defined __UCLIBC_HAS_RPC__
  int (*ptr_pthread_internal_tsd_set) (int key, const void *pointer);
  void * (*ptr_pthread_internal_tsd_get) (int key);
  void ** __attribute__ ((__const__))
    (*ptr_pthread_internal_tsd_address) (int key);
#endif
#if 0
  int (*ptr_pthread_sigaction) (int sig, const struct sigaction * act,
				struct sigaction *oact);
  int (*ptr_pthread_sigwait) (const sigset_t *set, int *sig);
  int (*ptr_pthread_raise) (int sig);
#endif
  int (*ptr_pthread_cond_timedwait) (pthread_cond_t *, pthread_mutex_t *,
				       const struct timespec *);
#if 0
  void (*ptr__pthread_cleanup_push) (struct _pthread_cleanup_buffer * buffer,
				     void (*routine)(void *), void * arg);
#endif
  void (*ptr__pthread_cleanup_push_defer) (struct _pthread_cleanup_buffer * buffer,
					   void (*routine)(void *), void * arg);
#if 0
  void (*ptr__pthread_cleanup_pop) (struct _pthread_cleanup_buffer * buffer,
				    int execute);
#endif
  void (*ptr__pthread_cleanup_pop_restore) (struct _pthread_cleanup_buffer * buffer,
					    int execute);
};

/* Variable in libc.so.  */
extern struct pthread_functions __libc_pthread_functions attribute_hidden;

extern int * __libc_pthread_init (const struct pthread_functions *functions);

#endif	/* pthread-functions.h */
