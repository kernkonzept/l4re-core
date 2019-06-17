/* $NetBSD: t_join.c,v 1.9 2017/07/02 16:41:33 joerg Exp $ */

/*-
 * Copyright (c) 2010 The NetBSD Foundation, Inc.
 * Copyright (C) 2019 Kernkonzept GmbH.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jukka Ruohonen.
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

#include <errno.h>
#include <pthread.h>
#include <stdint.h>

#define __arraycount(a) (sizeof(a) / sizeof(a[0]))

/*
 * L4 pthread enforces a 2MB limit for stack + guard size. With 25 threads the
 * test algorithm for picking the stack and guard sizes exceeded the 2MB limit
 * therefore the starting STACKSIZE is reduced from 64KB to 32KB.
 */
#define STACKSIZE 32768
#define NTHREADS 25

static bool error;

static void *
threadfunc1(void *);
static void *
threadfunc2(void *);

/**
 * Checks basic error conditions in thread join.
 */
TEST(Pthread, Join)
{
  pthread_t thread;

  ASSERT_EQ(0, pthread_create(&thread, NULL, threadfunc1, NULL));
  ASSERT_EQ(0, pthread_join(thread, NULL));
}

static void *
threadfunc1(void *arg)
{
  pthread_t thread[NTHREADS];
  pthread_t caller;
  void *val = NULL;
  uintptr_t i;
  int rv;
  pthread_attr_t attr;

  (void)arg;
  caller = pthread_self();

  /*
   * The behavior is undefined, but should error
   * out, if we try to join the calling thread.
   */
  rv = pthread_join(caller, NULL);

  /*
   * The specification recommends EDEADLK.
   */
  EXPECT_TRUE(rv != 0);
  EXPECT_EQ(EDEADLK, rv);

  EXPECT_TRUE(pthread_attr_init(&attr) == 0);

  for (i = 0; i < __arraycount(thread); i++)
    {
      error = true;

      /*
       * L4 pthread enforces guard size < stack size therefore switch up the
       * test to match the implementation.
       */
      EXPECT_TRUE(pthread_attr_setstacksize(&attr, STACKSIZE * (i + 2)) == 0);
      EXPECT_TRUE(pthread_attr_setguardsize(&attr, STACKSIZE * (i + 1)) == 0);

      rv = pthread_create(&thread[i], &attr, threadfunc2, (void *)i);

      EXPECT_EQ(0, rv);

      /*
       * Check join and exit condition.
       */
      EXPECT_EQ(0, pthread_join(thread[i], &val));

      EXPECT_EQ(false, error);

      EXPECT_TRUE(val != NULL);
      EXPECT_TRUE(val == (void *)(i + 1));
    }

  EXPECT_TRUE(pthread_attr_destroy(&attr) == 0);

  pthread_exit(NULL);

  return NULL;
}

static void *
threadfunc2(void *arg)
{
  static uintptr_t i = 0;
  uintptr_t j;
  pthread_attr_t attr;
  size_t stacksize, guardsize;

  j = (uintptr_t)arg;

  EXPECT_TRUE(pthread_getattr_np(pthread_self(), &attr) == 0);
  EXPECT_TRUE(pthread_attr_getstacksize(&attr, &stacksize) == 0);
  EXPECT_EQ(STACKSIZE * (j + 2), stacksize);
  EXPECT_TRUE(pthread_attr_getguardsize(&attr, &guardsize) == 0);
  EXPECT_EQ(STACKSIZE * (j + 1), guardsize);
  EXPECT_TRUE(pthread_attr_destroy(&attr) == 0);

  if (i++ == j)
    error = false;

  pthread_exit((void *)i);

  return NULL;
}
