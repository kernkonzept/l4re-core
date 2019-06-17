/* $NetBSD: t_barrier.c,v 1.2 2010/11/03 16:10:22 christos Exp $ */

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

#include <l4/atkins/tap/main>
#include <pthread-l4.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static Atkins::Dbg
trace()
{
  return Atkins::Dbg(Atkins::Dbg::Trace);
}

#define COUNT 5
#define SLEEP 1

static pthread_barrier_t barrier;
static pthread_mutex_t mutex;
static int serial_count;
static int after_barrier_count;

static void *
threadfunc(void *arg)
{
  int which = (int)(long)arg;
  int rv;

  trace().printf("thread %d entering barrier\n", which);
  rv = pthread_barrier_wait(&barrier);
  trace().printf("thread %d leaving barrier -> %d\n", which, rv);

  EXPECT_EQ(0, pthread_mutex_lock(&mutex));
  after_barrier_count++;
  if (rv == PTHREAD_BARRIER_SERIAL_THREAD)
    serial_count++;
  EXPECT_EQ(0, pthread_mutex_unlock(&mutex));

  return NULL;
}

/**
 * Checks barrier count is reached before threads continue execution.
 *
 * Multiple threads are started and each is expected to block on a
 * pthread_barrier_wait() call before proceeding.
 */
TEST(Pthread, Barrier)
{
  int i;
  pthread_t thr[COUNT];
  void *joinval;

  ASSERT_EQ(0, pthread_mutex_init(&mutex, NULL));
  ASSERT_EQ(0, pthread_barrier_init(&barrier, NULL, COUNT));

  for (i = 0; i < COUNT; i++)
    {
      ASSERT_EQ(0, pthread_mutex_lock(&mutex));
      ASSERT_EQ(0, after_barrier_count);
      ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
      ASSERT_EQ(0, pthread_create(&thr[i], NULL, threadfunc, (void *)(long)i));
      sleep(SLEEP);
    }

  for (i = 0; i < COUNT; i++)
    {
      ASSERT_EQ(0, pthread_join(thr[i], &joinval));
      ASSERT_EQ(0, pthread_mutex_lock(&mutex));
      ASSERT_TRUE(after_barrier_count > i);
      ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
      trace().printf("main joined with thread %d\n", i);
    }

  ASSERT_EQ(0, pthread_mutex_lock(&mutex));
  ASSERT_EQ(COUNT, after_barrier_count);
  ASSERT_EQ(0, pthread_mutex_unlock(&mutex));
  ASSERT_EQ(1, serial_count);
}
