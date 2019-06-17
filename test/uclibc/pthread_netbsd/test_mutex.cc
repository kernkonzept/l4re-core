/* $NetBSD: t_mutex.c,v 1.19 2017/12/01 13:25:29 kre Exp $ */

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

#include <sys/time.h> /* For timespecadd */
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/param.h>

#define ITERATIONS 2000000
#define UINT16_MAX 0xffff

static Atkins::Dbg
trace()
{
  return Atkins::Dbg(Atkins::Dbg::Trace);
}

/*
 * Structure for passing arguments between main test thread and helper thread.
 */
struct Test_args
{
  Test_args(pthread_mutex_t *m, struct timespec const *t)
  : mutex(m), ts(t), shared(0), count2(0)
  {}

  Test_args(pthread_mutex_t *m) : mutex(m), ts(nullptr), shared(0), count2(0) {}

  pthread_mutex_t *mutex;
  struct timespec const *ts;
  int shared;
  long count2;
};

/* This code is used for verifying non-timed specific code */
static struct timespec const ts_lengthy = {.tv_sec = UINT16_MAX, .tv_nsec = 0};
/* This code is used for verifying timed-only specific code */
static struct timespec const ts_shortlived = {.tv_sec = 0, .tv_nsec = 120};

static int
timed_mutex_lock(pthread_mutex_t *m, struct timespec const *ts)
{
  struct timespec ts_wait;
  EXPECT_TRUE(clock_gettime(CLOCK_REALTIME, &ts_wait) != -1);
  // timespecadd
  ts_wait.tv_sec += ts->tv_sec;
  ts_wait.tv_nsec += ts->tv_nsec;
  if (ts_wait.tv_nsec >= 1000000000)
    {
      ts_wait.tv_sec++;
      ts_wait.tv_nsec -= 1000000000;
    }

  return pthread_mutex_timedlock(m, &ts_wait);
}

static int
mutex_lock(pthread_mutex_t *m, struct timespec const *ts)
{
  if (ts)
    return timed_mutex_lock(m, ts);
  else
    return pthread_mutex_lock(m);
}

class PthreadTest : public ::testing::TestWithParam<struct timespec const *>
{};

static INSTANTIATE_TEST_CASE_P(Mutex, PthreadTest,
                               testing::Values(nullptr, &ts_lengthy));

static void *
mutex1_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  trace().printf("2: Second thread.\n");

  trace().printf("2: Locking mutex\n");
  EXPECT_EQ(0, mutex_lock(a->mutex, a->ts));
  trace().printf("2: Got mutex. *param = %d\n", a->shared);
  EXPECT_EQ(20, a->shared);
  a->shared++;
  EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));

  return &a->shared;
}

/**
 * Check mutex acquire and release between threads.
 */
TEST_P(PthreadTest, Mutex1)
{
  pthread_t thr;
  void *joinval;
  pthread_mutex_t mutex;
  Test_args arg{&mutex, GetParam()};

  trace().printf("1: Mutex-test 1\n");

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));
  arg.shared = 1;
  ASSERT_EQ(0, mutex_lock(&mutex, arg.ts));
  ASSERT_EQ(0, pthread_create(&thr, NULL, mutex1_threadfunc, &arg));
  trace().printf("1: Before changing the value.\n");
  sleep(2);
  arg.shared = 20;
  trace().printf("1: Before releasing the mutex.\n");
  sleep(2);
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
  trace().printf("1: After releasing the mutex.\n");
  ASSERT_EQ(0, pthread_join(thr, &joinval));

  ASSERT_EQ(0, mutex_lock(&mutex, arg.ts));
  trace().printf(
    "1: Thread joined. Shared value was %d. Return value (int) was %d\n",
    arg.shared, *(int *)joinval);
  ASSERT_EQ(21, arg.shared);
  ASSERT_EQ(21, *(int *)joinval);
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
}

static void *
mutex2_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  trace().printf("2: Second thread (%p). Count is %ld\n", pthread_self(),
                 a->count2);

  while (a->count2--)
    {
      EXPECT_EQ(0, mutex_lock(a->mutex, a->ts));
      a->shared++;
      EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));
    }

  return (void *)a->count2;
}

/**
 * Check mutex acquire and release over many cycles.
 */
TEST_P(PthreadTest, Mutex2)
{
  long count;
  pthread_t thr;
  void *joinval;
  pthread_mutex_t mutex;
  Test_args arg{&mutex, GetParam()};

  trace().printf("1: Mutex-test 2\n");

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));

  arg.shared = 0;
  count = arg.count2 = ITERATIONS;

  ASSERT_EQ(0, mutex_lock(&mutex, arg.ts));
  ASSERT_EQ(0, pthread_create(&thr, NULL, mutex2_threadfunc, &arg));

  trace().printf("1: Thread %p\n", pthread_self());

  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));

  while (count--)
    {
      ASSERT_EQ(0, mutex_lock(&mutex, arg.ts));
      arg.shared++;
      ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
    }

  ASSERT_EQ(0, pthread_join(thr, &joinval));

  ASSERT_EQ(0, mutex_lock(&mutex, arg.ts));
  trace().printf(
    "1: Thread joined. Shared value was %d. Return value (long) was %ld\n",
    arg.shared, (long)joinval);
  ASSERT_EQ(2 * ITERATIONS, arg.shared);
}

static void *
mutex3_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  trace().printf("2: Second thread (%p). Count is %ld\n", pthread_self(),
                 a->count2);

  while (a->count2--)
    {
      EXPECT_EQ(0, mutex_lock(a->mutex, a->ts));
      a->shared++;
      EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));
    }

  return (void *)a->count2;
}

/**
 * Check statically initialized mutex acquire and release over many cycles.
 */
TEST_P(PthreadTest, Mutex3)
{
  long count;
  pthread_t thr;
  void *joinval;
  pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
  Test_args arg{&static_mutex, GetParam()};

  trace().printf("1: Mutex-test 3\n");

  arg.shared = 0;
  count = arg.count2 = ITERATIONS;

  ASSERT_EQ(0, mutex_lock(&static_mutex, arg.ts));
  ASSERT_EQ(0, pthread_create(&thr, NULL, mutex3_threadfunc, &arg));

  trace().printf("1: Thread %p\n", pthread_self());

  ASSERT_EQ(0, pthread_mutex_unlock(&static_mutex));

  while (count--)
    {
      ASSERT_EQ(0, mutex_lock(&static_mutex, arg.ts));
      arg.shared++;
      ASSERT_EQ(0, pthread_mutex_unlock(&static_mutex));
    }

  ASSERT_EQ(0, pthread_join(thr, &joinval));

  ASSERT_EQ(0, mutex_lock(&static_mutex, arg.ts));
  trace().printf(
    "1: Thread joined. Shared value was %d. Return value (long) was %ld\n",
    arg.shared, (long)joinval);
  ASSERT_EQ(2 * ITERATIONS, arg.shared);
}

static void *
mutex4_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  trace().printf("2: Second thread.\n");

  trace().printf("2: Locking mutex\n");
  EXPECT_EQ(0, mutex_lock(a->mutex, a->ts));
  trace().printf("2: Got mutex. *param = %d\n", a->shared);
  a->shared++;

  EXPECT_EQ(0, pthread_mutex_unlock(a->mutex));

  return &a->shared;
}

/**
 * Check recursive mutex acquire and release.
 */
TEST_P(PthreadTest, Mutex4)
{
  pthread_t thr;
  pthread_mutexattr_t mattr;
  void *joinval;
  pthread_mutex_t mutex;
  Test_args arg{&mutex, GetParam()};

  trace().printf("1: Mutex-test 4\n");

  ASSERT_EQ(0, pthread_mutexattr_init(&mattr));
  ASSERT_EQ(0, pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE));

  ASSERT_EQ(0, pthread_mutex_init(&mutex, &mattr));

  ASSERT_EQ(0, pthread_mutexattr_destroy(&mattr));

  arg.shared = 1;
  ASSERT_EQ(0, mutex_lock(&mutex, arg.ts));
  ASSERT_EQ(0, pthread_create(&thr, NULL, mutex4_threadfunc, &arg));

  trace().printf("1: Before recursively acquiring the mutex.\n");
  ASSERT_EQ(0, mutex_lock(&mutex, arg.ts));

  trace().printf("1: Before releasing the mutex once.\n");
  sleep(2);
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
  trace().printf("1: After releasing the mutex once.\n");

  arg.shared = 20;

  trace().printf("1: Before releasing the mutex twice.\n");
  sleep(2);
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
  trace().printf("1: After releasing the mutex twice.\n");

  ASSERT_EQ(0, pthread_join(thr, &joinval));

  ASSERT_EQ(0, mutex_lock(&mutex, arg.ts));
  trace().printf(
    "1: Thread joined. Shared value was %d. Return value (int) was %d\n",
    arg.shared, *(int *)joinval);
  ASSERT_EQ(21, arg.shared);
  ASSERT_EQ(21, *(int *)joinval);
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
}

/**
 * Checks timeout on selflock.
 */
TEST(Pthread, TimedMutex1)
{
  pthread_mutex_t mutex;

  trace().printf("Timed mutex-test 1\n");

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));

  trace().printf("Before acquiring mutex\n");
  ASSERT_EQ(0, pthread_mutex_lock(&mutex));

  trace().printf(
    "Before endeavor to reacquire timed-mutex (timeout expected)\n");
  ASSERT_EQ(ETIMEDOUT, mutex_lock(&mutex, &ts_shortlived));

  trace().printf("Unlocking mutex\n");
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
}

/**
 * Checks timeout on selflock with timedlock.
 */
TEST(Pthread, TimedMutex2)
{
  pthread_mutex_t mutex;

  trace().printf("Timed mutex-test 2\n");

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));

  trace().printf("Before acquiring mutex with timedlock\n");
  ASSERT_EQ(0, mutex_lock(&mutex, &ts_lengthy));

  trace().printf(
    "Before endeavor to reacquire timed-mutex (timeout expected)\n");
  ASSERT_EQ(ETIMEDOUT, mutex_lock(&mutex, &ts_shortlived));

  trace().printf("Unlocking mutex\n");
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
}

static void *
timedmtx_thrdfunc(void *arg)
{
  pthread_mutex_t *m = reinterpret_cast<pthread_mutex_t *>(arg);

  trace().printf(
    "Before endeavor to reacquire timed-mutex (timeout expected)\n");
  EXPECT_EQ(ETIMEDOUT, mutex_lock(m, &ts_shortlived));

  return NULL;
}

/**
 * Checks timeout on selflock in a new thread.
 */
TEST(Pthread, TimedMutex3)
{
  pthread_mutex_t mutex;
  pthread_t thr;

  trace().printf("Timed mutex-test 3\n");

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));

  trace().printf("Before acquiring mutex with timedlock\n");
  ASSERT_EQ(0, pthread_mutex_lock(&mutex));

  trace().printf("Before creating thr thread\n");
  ASSERT_EQ(0, pthread_create(&thr, NULL, timedmtx_thrdfunc, &mutex));

  trace().printf("Before joining the mutex\n");
  ASSERT_EQ(0, pthread_join(thr, NULL));

  trace().printf("Unlocking mutex\n");
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
}

/**
 * Checks timeout on selflock with timedlock in a new thread.
 */
TEST(Pthread, TimedMutex4)
{
  pthread_mutex_t mutex;
  pthread_t thr;

  trace().printf("Timed mutex-test 4\n");

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));

  trace().printf("Before acquiring mutex with timedlock\n");
  ASSERT_EQ(0, mutex_lock(&mutex, &ts_lengthy));

  trace().printf("Before creating thr thread\n");
  ASSERT_EQ(0, pthread_create(&thr, NULL, timedmtx_thrdfunc, &mutex));

  trace().printf("Before joining the mutex\n");
  ASSERT_EQ(0, pthread_join(thr, NULL));

  trace().printf("Unlocking mutex\n");
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
}
