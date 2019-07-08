/* $NetBSD: t_sem.c,v 1.9 2017/01/16 16:22:22 christos Exp $ */

/*
 * Copyright (c) 2008, 2010 The NetBSD Foundation, Inc.
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

/*-
 * Copyright (c)2004 YAMAMOTO Takashi,
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/****************************************************************************
 *
 * Copyright (C) 2000 Jason Evans <jasone@freebsd.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <pthread-l4.h>
#include <l4/atkins/tap/main>

#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static Atkins::Dbg
trace()
{
  return Atkins::Dbg(Atkins::Dbg::Trace);
}

static struct timespec const ts_shortlived = {.tv_sec = 0, .tv_nsec = 120};

#define NTHREADS 10

/**
 * Named semaphores are not available.
 */
TEST(Pthread, NamedSemaphores)
{
  errno = 0;
  EXPECT_EQ(-1, sem_unlink("/foo")) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";

  errno = 0;
  EXPECT_EQ(SEM_FAILED, sem_open("/foo", O_CREAT | O_EXCL, 0644, 0))
    << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";

  errno = 0;
  EXPECT_EQ(-1, sem_close(nullptr)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";

  errno = 0;
  EXPECT_EQ(-1, sem_unlink("/foo")) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

static void *
entry(void *a_arg)
{
  pthread_t self = pthread_self();
  sem_t *semp = (sem_t *)a_arg;

  trace().printf("Thread %p waiting for semaphore...\n", self);
  sem_wait(semp);
  trace().printf("Thread %p got semaphore\n", self);

  return NULL;
}

/**
 * Checks unnamed semaphores.
 */
TEST(Pthread, UnnamedSemaphores)
{
  sem_t sem_a, sem_b;
  pthread_t threads[NTHREADS];
  unsigned i, j;
  int val;

  trace().printf("Test begin\n");

  ASSERT_EQ(0, sem_init(&sem_b, 0, 0));
  ASSERT_EQ(0, sem_getvalue(&sem_b, &val));
  EXPECT_EQ(0, val);

  ASSERT_EQ(0, sem_post(&sem_b));
  ASSERT_EQ(0, sem_getvalue(&sem_b, &val));
  EXPECT_EQ(1, val);

  ASSERT_EQ(0, sem_wait(&sem_b));
  EXPECT_EQ(-1, sem_trywait(&sem_b));
  EXPECT_EQ(EAGAIN, errno);
  EXPECT_EQ(-1, sem_timedwait(&sem_b, &ts_shortlived));
  EXPECT_EQ(ETIMEDOUT, errno);
  ASSERT_EQ(0, sem_post(&sem_b));
  ASSERT_EQ(0, sem_trywait(&sem_b));
  ASSERT_EQ(0, sem_post(&sem_b));
  ASSERT_EQ(0, sem_wait(&sem_b));
  ASSERT_EQ(0, sem_post(&sem_b));

  ASSERT_EQ(0, sem_destroy(&sem_b));

  ASSERT_EQ(0, sem_init(&sem_a, 0, 0));

  for (j = 0; j < 2; j++)
    {
      for (i = 0; i < NTHREADS; i++)
        {
          ASSERT_EQ(0,
                    pthread_create(&threads[i], NULL, entry, (void *)&sem_a));
        }

      for (i = 0; i < NTHREADS; i++)
        {
          usleep(10000);
          trace().printf("main loop %u: posting...\n", j + 1);
          ASSERT_EQ(0, sem_post(&sem_a));
        }

      for (i = 0; i < NTHREADS; i++)
        {
          ASSERT_EQ(0, pthread_join(threads[i], NULL));
        }
    }

  ASSERT_EQ(0, sem_destroy(&sem_a));

  trace().printf("Test end\n");
}
