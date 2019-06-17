/* $NetBSD: t_rwlock.c,v 1.2 2015/06/26 11:07:20 pooka Exp $ */

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

#include <pthread-l4.h>
#include <l4/atkins/tap/main>

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static pthread_rwlock_t lk;

static struct timespec to;

static pthread_rwlock_t static_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/* ARGSUSED */
static void *
do_nothing(void *dummy)
{
  (void)dummy;

  return NULL;
}

/**
 * Checks read/write locks.
 */
TEST(Pthread, RWLock)
{
  int error;
  pthread_t t;

  ASSERT_EQ(0, pthread_create(&t, NULL, do_nothing, NULL));
  ASSERT_EQ(0, pthread_rwlock_init(&lk, NULL));
  ASSERT_EQ(0, pthread_rwlock_rdlock(&lk));
  ASSERT_EQ(0, pthread_rwlock_rdlock(&lk));
  ASSERT_EQ(0, pthread_rwlock_unlock(&lk));

  ASSERT_EQ(EBUSY, pthread_rwlock_trywrlock(&lk));

  ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME, &to)) << strerror(errno);
  to.tv_sec++;
  error = pthread_rwlock_timedwrlock(&lk, &to);
  ASSERT_TRUE(error == ETIMEDOUT || error == EDEADLK) << strerror(error);

  ASSERT_EQ(0, pthread_rwlock_unlock(&lk));

  ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME, &to)) << strerror(errno);
  to.tv_sec++;
  ASSERT_EQ(0, pthread_rwlock_timedwrlock(&lk, &to));

  ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME, &to)) << strerror(errno);
  to.tv_sec++;
  error = pthread_rwlock_timedwrlock(&lk, &to);
  ASSERT_TRUE(error == ETIMEDOUT || error == EDEADLK) << strerror(error);
}

/**
 * Checks read/write locks with static initialization.
 */
TEST(Pthread, RWLockStatic)
{

  ASSERT_EQ(0, pthread_rwlock_rdlock(&static_rwlock));
  ASSERT_EQ(0, pthread_rwlock_unlock(&static_rwlock));
  ASSERT_EQ(0, pthread_rwlock_destroy(&static_rwlock));
}
