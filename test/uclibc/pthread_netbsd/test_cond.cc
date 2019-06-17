/* $NetBSD: t_cond.c,v 1.7 2016/07/03 14:24:59 christos Exp $ */

/*
 * Copyright (c) 2008 The NetBSD Foundation, Inc.
 * Copyright (C) 2019 Kernkonzept GmbH.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <pthread-l4.h>
#include <l4/atkins/tap/main>

#include <sys/time.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static Atkins::Dbg
trace()
{
  return Atkins::Dbg(Atkins::Dbg::Trace);
}

#define __arraycount(a) (sizeof(a) / sizeof(a[0]))

/*
 * Structure for passing arguments between main test thread and helper thread.
 */
struct Test_args
{
  Test_args(pthread_mutex_t *m, pthread_cond_t *c)
  : mutex(m), cond(c), shared(0), count(0), total(0), toggle(0)
  {}

  pthread_mutex_t *mutex;
  pthread_cond_t *cond;
  int shared;
  int count;
  int total;
  int toggle;
};

static void *
signal_delay_wait_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  trace().printf("2: Second thread.\n");

  trace().printf("2: Locking mutex\n");
  EXPECT_EQ(0, pthread_mutex_lock(a->mutex));
  trace().printf("2: Got mutex.\n");
  trace().printf("Shared value: %d. Changing to 0.\n", a->shared);
  a->shared = 0;

  EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));
  EXPECT_EQ(0, pthread_cond_signal(a->cond));

  return NULL;
}

/**
 * Checks condition variables: signal after a delay.
 */
TEST(Pthread, CondSignalDelayWait)
{
  pthread_t thr;
  void *joinval;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  Test_args arg{&mutex, &cond};

  trace().printf("1: condition variable test 1\n");

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));
  ASSERT_EQ(0, pthread_cond_init(&cond, NULL));

  ASSERT_EQ(0, pthread_mutex_lock(&mutex));

  arg.shared = 1;

  ASSERT_EQ(0, pthread_create(&thr, NULL, signal_delay_wait_threadfunc, &arg));

  trace().printf("1: Before waiting.\n");
  do
    {
      sleep(2);
      ASSERT_EQ(0, pthread_cond_wait(&cond, &mutex));
      trace().printf("1: After waiting, in loop.\n");
    }
  while (arg.shared != 0);

  trace().printf("1: After the loop.\n");

  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));

  trace().printf("1: After releasing the mutex.\n");
  ASSERT_EQ(0, pthread_join(thr, &joinval));

  trace().printf("1: Thread joined.\n");
}

static void *
signal_before_unlock_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  trace().printf("2: Second thread.\n");

  trace().printf("2: Locking mutex\n");
  EXPECT_EQ(0, pthread_mutex_lock(a->mutex));
  trace().printf("2: Got mutex.\n");
  trace().printf("Shared value: %d. Changing to 0.\n", a->shared);
  a->shared = 0;

  /* Signal first, then unlock, for a different test than #1. */
  EXPECT_EQ(0, pthread_cond_signal(a->cond));
  EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));

  return NULL;
}

/**
 * Checks condition variables: signal before unlocking mutex.
 */
TEST(Pthread, CondSignalBeforeUnlock)
{
  pthread_t thr;
  void *joinval;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  Test_args arg{&mutex, &cond};

  trace().printf("1: condition variable test 2\n");

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));
  ASSERT_EQ(0, pthread_cond_init(&cond, NULL));

  ASSERT_EQ(0, pthread_mutex_lock(&mutex));

  arg.shared = 1;

  ASSERT_EQ(0,
            pthread_create(&thr, NULL, signal_before_unlock_threadfunc, &arg));

  trace().printf("1: Before waiting.\n");
  do
    {
      sleep(2);
      ASSERT_EQ(0, pthread_cond_wait(&cond, &mutex));
      trace().printf("1: After waiting, in loop.\n");
    }
  while (arg.shared != 0);

  trace().printf("1: After the loop.\n");

  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));

  trace().printf("1: After releasing the mutex.\n");
  ASSERT_EQ(0, pthread_join(thr, &joinval));

  trace().printf("1: Thread joined.\n");
}

static void *
signal_before_unlock_static_init_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  trace().printf("2: Second thread.\n");

  trace().printf("2: Locking mutex\n");
  EXPECT_EQ(0, pthread_mutex_lock(a->mutex));
  trace().printf("2: Got mutex.\n");
  trace().printf("Shared value: %d. Changing to 0.\n", a->shared);
  a->shared = 0;

  /* Signal first, then unlock, for a different test than #1. */
  EXPECT_EQ(0, pthread_cond_signal(a->cond));
  EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));

  return NULL;
}

/**
 * Checks condition variables: signal before unlocking mutex, use static
 * initializers.
 */
TEST(Pthread, CondSignalBeforeUnlockStaticInit)
{
  pthread_t thr;
  void *joinval;
  pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t static_cond = PTHREAD_COND_INITIALIZER;
  Test_args arg{&static_mutex, &static_cond};

  trace().printf("1: condition variable test 3\n");

  ASSERT_EQ(0, pthread_mutex_lock(&static_mutex));

  arg.shared = 1;

  ASSERT_EQ(0,
            pthread_create(&thr, NULL,
                           signal_before_unlock_static_init_threadfunc, &arg));

  trace().printf("1: Before waiting.\n");
  do
    {
      sleep(2);
      ASSERT_EQ(0, pthread_cond_wait(&static_cond, &static_mutex));
      trace().printf("1: After waiting, in loop.\n");
    }
  while (arg.shared != 0);

  trace().printf("1: After the loop.\n");

  ASSERT_EQ(0, pthread_mutex_unlock(&static_mutex));

  trace().printf("1: After releasing the mutex.\n");
  ASSERT_EQ(0, pthread_join(thr, &joinval));

  trace().printf("1: Thread joined.\n");
}

static void *
signal_wait_race_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  trace().printf("2: Second thread.\n");
  EXPECT_EQ(0, pthread_mutex_lock(a->mutex));
  trace().printf("2: Before the loop.\n");
  while (a->count > 0)
    {
      a->count--;
      a->total++;
      a->toggle = 0;
      /* trace().printf("2: Before signal %d.\n", count); */
      EXPECT_EQ(0, pthread_cond_signal(a->cond));
      do
        {
          EXPECT_EQ(0, pthread_cond_wait(a->cond, a->mutex));
        }
      while (a->toggle != 1);
    }
  trace().printf("2: After the loop.\n");
  EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));

  return NULL;
}

/**
 * Checks condition variables for race condition surrounding waits.
 */
TEST(Pthread, CondSignalWaitRace)
{
  pthread_t thr;
  void *joinval;
  pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t static_cond = PTHREAD_COND_INITIALIZER;
  Test_args arg{&static_mutex, &static_cond};

  trace().printf("1: condition variable test 4\n");

  ASSERT_EQ(0, pthread_mutex_lock(&static_mutex));

  arg.count = 50000;
  arg.total = 0;
  arg.toggle = 0;

  ASSERT_EQ(0, pthread_create(&thr, NULL, signal_wait_race_threadfunc, &arg));

  trace().printf("1: Before waiting.\n");
  while (arg.count > 0)
    {
      arg.count--;
      arg.total++;
      arg.toggle = 1;
      /* trace().printf("1: Before signal %d.\n", count); */
      ASSERT_EQ(0, pthread_cond_signal(&static_cond));
      do
        {
          ASSERT_EQ(0, pthread_cond_wait(&static_cond, &static_mutex));
        }
      while (arg.toggle != 0);
    }
  trace().printf("1: After the loop.\n");

  arg.toggle = 1;
  ASSERT_EQ(0, pthread_mutex_unlock(&static_mutex));
  ASSERT_EQ(0, pthread_cond_signal(&static_cond));

  trace().printf("1: After releasing the mutex.\n");
  ASSERT_EQ(0, pthread_join(thr, &joinval));

  trace().printf("1: Thread joined. Final count = %d, total = %d\n", arg.count,
                 arg.total);

  ASSERT_EQ(0, arg.count);
  ASSERT_EQ(50000, arg.total);
}

static void *
pthread_cond_timedwait_func(void *arg)
{
  pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t static_cond = PTHREAD_COND_INITIALIZER;
  struct timespec ts;
  size_t i = 0;
  int rv;

  (void)arg;

  for (;;)
    {
      if (i++ >= 10000)
        pthread_exit(NULL);

      (void)memset(&ts, 0, sizeof(struct timespec));

      /*
       * In case clock_gettime starts at 0 ensure we don't wrap around
       * backwards when we set time in the past.
       */
      do
        {
          EXPECT_TRUE(clock_gettime(CLOCK_REALTIME, &ts) == 0);
        }
      while (ts.tv_sec < 3);

      /*
       * Set to one second in the past:
       * pthread_cond_timedwait(3) should
       * return ETIMEDOUT immediately.
       */
      ts.tv_sec = ts.tv_sec - 1;

      EXPECT_EQ(0, pthread_mutex_lock(&static_mutex));
      rv = pthread_cond_timedwait(&static_cond, &static_mutex, &ts);

      /*
       * Sometimes we catch ESRCH.
       * This should never happen.
       */
      EXPECT_TRUE(rv == ETIMEDOUT);
      EXPECT_EQ(0, pthread_mutex_unlock(&static_mutex));
    }
}

/**
 * Checks condition variables for race condition surrounding timed waits.
 */
TEST(Pthread, CondSignalTimedWaitRace)
{
  pthread_t tid[64];
  size_t i;

  for (i = 0; i < __arraycount(tid); i++)
    {
      ASSERT_EQ(0, pthread_create(&tid[i], NULL, pthread_cond_timedwait_func,
                                  NULL));
    }

  for (i = 0; i < __arraycount(tid); i++)
    {
      ASSERT_EQ(0, pthread_join(tid[i], NULL));
    }
}

static void *
broadcast_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  trace().printf("2: Second thread.\n");

  EXPECT_EQ(0, pthread_mutex_lock(a->mutex));
  while (a->count > 0)
    {
      a->count--;
      a->total++;
      a->toggle = 0;
      EXPECT_EQ(0, pthread_cond_signal(a->cond));
      do
        {
          EXPECT_EQ(0, pthread_cond_wait(a->cond, a->mutex));
        }
      while (a->toggle != 1);
    }
  trace().printf("2: After the loop.\n");
  EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));

  return NULL;
}

/**
 * Checks condition variable signal can be broadcast.
 */
TEST(Pthread, CondSignalBroadcast)
{
  pthread_t thr;
  void *joinval;
  pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t static_cond = PTHREAD_COND_INITIALIZER;
  Test_args arg{&static_mutex, &static_cond};

  trace().printf("1: condition variable test 5\n");

  ASSERT_EQ(0, pthread_mutex_lock(&static_mutex));

  arg.count = 50000;
  arg.total = 0;
  arg.toggle = 0;

  ASSERT_EQ(0, pthread_create(&thr, NULL, broadcast_threadfunc, &arg));

  trace().printf("1: Before waiting.\n");
  while (arg.count > 0)
    {
      arg.count--;
      arg.total++;
      arg.toggle = 1;
      ASSERT_EQ(0, pthread_cond_broadcast(&static_cond));
      do
        {
          ASSERT_EQ(0, pthread_cond_wait(&static_cond, &static_mutex));
        }
      while (arg.toggle != 0);
    }
  trace().printf("1: After the loop.\n");

  arg.toggle = 1;
  ASSERT_EQ(0, pthread_mutex_unlock(&static_mutex));
  ASSERT_EQ(0, pthread_cond_signal(&static_cond));

  trace().printf("1: After releasing the mutex.\n");
  ASSERT_EQ(0, pthread_join(thr, &joinval));

  trace().printf("1: Thread joined. Final count = %d, total = %d\n", arg.count,
                 arg.total);

  ASSERT_EQ(0, arg.count);
  ASSERT_EQ(50000, arg.total);
}

static void *
bogus_timedwaits_threadfunc(void *arg)
{
  (void)arg;

  return NULL;
}

/**
 * Checks condition variables: bogus timedwaits are handled.
 */
TEST(Pthread, CondBogusTimedWait)
{
  pthread_t thr;
  struct timespec ts;
  struct timeval tv;
  pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t static_cond = PTHREAD_COND_INITIALIZER;

  trace().printf("condition variable test 6: bogus timedwaits\n");

  ASSERT_EQ(0, pthread_mutex_lock(&static_mutex));

  trace().printf("unthreaded test (past)\n");

  /*
   * In case timeofday starts at 0 ensure we don't wrap around backwards when
   * we set time in the past.
   */
  do
    {
      gettimeofday(&tv, NULL);
    }
  while (tv.tv_sec < 3);

  tv.tv_sec -= 2; /* Place the time in the past */
  TIMEVAL_TO_TIMESPEC(&tv, &ts);

  ASSERT_EQ(ETIMEDOUT, pthread_cond_timedwait(&static_cond, &static_mutex, &ts))
    << "pthread_cond_timedwait() (unthreaded) in the past";

  trace().printf("unthreaded test (zero time)\n");
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  TIMEVAL_TO_TIMESPEC(&tv, &ts);

  ASSERT_EQ(ETIMEDOUT, pthread_cond_timedwait(&static_cond, &static_mutex, &ts))
    << "pthread_cond_timedwait() (unthreaded) with zero time";

  ASSERT_EQ(0, pthread_create(&thr, NULL, bogus_timedwaits_threadfunc, NULL));
  ASSERT_EQ(0, pthread_join(thr, NULL));

  trace().printf("threaded test\n");

  /*
   * In case timeofday starts at 0 ensure we don't wrap around backwards when
   * we set time in the past.
   */
  do
    {
      gettimeofday(&tv, NULL);
    }
  while (tv.tv_sec < 3);

  tv.tv_sec -= 2; /* Place the time in the past */
  TIMEVAL_TO_TIMESPEC(&tv, &ts);

  ASSERT_EQ(ETIMEDOUT, pthread_cond_timedwait(&static_cond, &static_mutex, &ts))
    << "pthread_cond_timedwait() (threaded) in the past";

  trace().printf("threaded test (zero time)\n");
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  TIMEVAL_TO_TIMESPEC(&tv, &ts);

  ASSERT_EQ(ETIMEDOUT, pthread_cond_timedwait(&static_cond, &static_mutex, &ts))
    << "pthread_cond_timedwait() (threaded) with zero time";

  ASSERT_EQ(0, pthread_mutex_unlock(&static_mutex));
}

static void
unlock(void *arg)
{
  pthread_mutex_unlock((pthread_mutex_t *)arg);
}

static void *
destroy_after_cancel_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  EXPECT_EQ(0, pthread_mutex_lock(a->mutex));

  pthread_cleanup_push(unlock, a->mutex);

  while (1)
    {
      a->shared = 1;
      EXPECT_EQ(0, pthread_cond_broadcast(a->cond));
      EXPECT_EQ(0, pthread_cond_wait(a->cond, a->mutex));
    }

  pthread_cleanup_pop(0);
  EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));

  return NULL;
}

/**
 * Checks destroying a condition variable after cancelling a wait.
 */
TEST(Pthread, DestroyAfterCancel)
{
  pthread_t thread;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  Test_args arg{&mutex, &cond};

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));
  ASSERT_EQ(0, pthread_cond_init(&cond, NULL));
  ASSERT_EQ(0, pthread_mutex_lock(&mutex));
  ASSERT_EQ(0, pthread_create(&thread, NULL, destroy_after_cancel_threadfunc,
                              &arg));

  while (arg.shared == 0)
    {
      ASSERT_EQ(0, pthread_cond_wait(&cond, &mutex));
    }

  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
  ASSERT_EQ(0, pthread_cancel(thread));

  ASSERT_EQ(0, pthread_join(thread, NULL));
  ASSERT_EQ(0, pthread_cond_destroy(&cond));

  ASSERT_EQ(0, pthread_mutex_destroy(&mutex));
}
