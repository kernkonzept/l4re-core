/* $NetBSD: t_once.c,v 1.2 2017/08/25 22:59:47 ginsbach Exp $ */

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
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static Atkins::Dbg
trace()
{
  return Atkins::Dbg(Atkins::Dbg::Trace);
}

static int x1;
static int x2;

#define NTHREADS 25

/*
 * Structure for passing arguments between main test thread and helper thread.
 */
struct Test_args
{
  Test_args(pthread_once_t *o, pthread_mutex_t *m) : once(o), mutex(m) {}
  Test_args(pthread_once_t *o) : once(o), mutex(nullptr) {}

  pthread_once_t *once;
  pthread_mutex_t *mutex;
};

static void
ofunc()
{
  trace().printf("Variable x has value %d\n", x1);
  x1++;
}

/**
 * Initialization function is called once when executed from same thread.
 */
TEST(Pthread, OnceSameThread)
{
  pthread_once_t once = PTHREAD_ONCE_INIT;

  x1 = 0;
  trace().printf("1: Test 1 of pthread_once()\n");

  ASSERT_EQ(0, pthread_once(&once, ofunc));
  ASSERT_EQ(0, pthread_once(&once, ofunc));

  trace().printf("1: X has value %d\n", x1);
  ASSERT_EQ(1, x1);
}

static void
once2_ofunc()
{
  x2++;
  trace().printf("ofunc: Variable x has value %d\n", x2);
  x2++;
}

static void *
once2_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  EXPECT_EQ(0, pthread_once(a->once, once2_ofunc));

  trace().printf("Thread sees x with value %d\n", x2);
  EXPECT_EQ(2, x2);

  return NULL;
}

/**
 * Initialization function is called once when executed from different threads.
 */
TEST(Pthread, OnceDifferentThreads)
{
  pthread_t threads[NTHREADS];
  int i;
  pthread_once_t once = PTHREAD_ONCE_INIT;
  Test_args arg{&once};

  x2 = 0;
  trace().printf("2: Test 2 of pthread_once()\n");

  for (i = 0; i < NTHREADS; i++)
    {
      ASSERT_EQ(0, pthread_create(&threads[i], NULL, once2_threadfunc, &arg));
    }

  for (i = 0; i < NTHREADS; i++)
    ASSERT_EQ(0, pthread_join(threads[i], NULL));

  trace().printf("2: X has value %d\n", x2);
  ASSERT_EQ(2, x2);
}

static void
once3_cleanup(void *m)
{
  pthread_mutex_t *mu = reinterpret_cast<pthread_mutex_t *>(m);

  ASSERT_EQ(0, pthread_mutex_unlock(mu));
}

static void
once3_ofunc()
{
  pthread_testcancel();
}

static void *
once3_threadfunc(void *arg)
{
  Test_args *a = reinterpret_cast<Test_args *>(arg);

  EXPECT_EQ(0, pthread_mutex_lock(a->mutex));
  pthread_cleanup_push(once3_cleanup, a->mutex);
  EXPECT_EQ(0, pthread_once(a->once, once3_ofunc));
  pthread_cleanup_pop(1);

  return NULL;
}

/**
 * Cleanup function is only called once when thread is cancelled.
 */
TEST(Pthread, OnceCleanup)
{
  pthread_t thread;
  pthread_once_t once = PTHREAD_ONCE_INIT;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  Test_args arg{&once, &mutex};

  ASSERT_EQ(0, pthread_mutex_lock(&mutex));
  ASSERT_EQ(0, pthread_create(&thread, NULL, once3_threadfunc, &arg));
  ASSERT_EQ(0, pthread_cancel(thread));
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
  ASSERT_EQ(0, pthread_join(thread, NULL));

  ASSERT_EQ(0, pthread_once(&once, ofunc));

  trace().printf("Test succeeded\n");
}
