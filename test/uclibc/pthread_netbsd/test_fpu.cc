/* $NetBSD: t_fpu.c,v 1.3 2017/01/16 16:27:43 christos Exp $ */

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

/*
 * This is adapted from part of csw/cstest of the MPD implementation by
 * the University of Arizona CS department (http://www.cs.arizona.edu/sr/)
 * which is in the public domain:
 *
 * "The MPD system is in the public domain and you may use and distribute it
 *  as you wish.  We ask that you retain credits referencing the University
 *  of Arizona and that you identify any changes you make.
 *
 *  We can't provide a warranty with MPD; it's up to you to determine its
 *  suitability and reliability for your needs.  We would like to hear of
 *  any problems you encounter but we cannot promise a timely correction."
 *
 * It was changed to use pthread_create() and sched_yield() instead of
 * the internal MPD context switching primitives by Ignatios Souvatzis
 * <is@netbsd.org>.
 */

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static Atkins::Dbg
trace()
{
  return Atkins::Dbg(Atkins::Dbg::Trace);
}

#define N_RECURSE 10

static void
recurse();

int recursion_depth = 0;
pthread_mutex_t recursion_depth_lock;

static void *
stir(void *p)
{
  double *q = (double *)p;
  double x = *q++;
  double y = *q++;
  double z = *q++;

  for (;;)
    {
      x = sin((y = cos(x + y + .4)) - (z = cos(x + z + .6)));
      EXPECT_EQ(0, sched_yield()) << "sched_yield failed: " << strerror(errno);
    }

  return NULL;
}

static double
mul3(double x, double y, double z)
{
  EXPECT_EQ(0, sched_yield()) << "sched_yield failed: " << strerror(errno);

  return x * y * z;
}

static void *
bar(void *p)
{
  double d;
  int rc;

  (void)p;

  d = mul3(mul3(2., 3., 5.), mul3(7., 11., 13.), mul3(17., 19., 23.));
  EXPECT_DOUBLE_EQ(223092870., d);

  EXPECT_EQ(0, pthread_mutex_lock(&recursion_depth_lock));
  rc = recursion_depth++;
  EXPECT_EQ(0, pthread_mutex_unlock(&recursion_depth_lock));

  if (rc < N_RECURSE)
    recurse();
  else
    SUCCEED();

  return NULL;
}

static void
recurse()
{
  pthread_t s2;
  ASSERT_EQ(0, pthread_create(&s2, 0, bar, 0));
  ASSERT_EQ(0, pthread_join(s2, NULL));
}

/**
 * Thread context switches leave the floating point computations unharmed.
 */
TEST(Pthread, Fpu)
{
  double stirseed[] = {1.7, 3.2, 2.4};
  pthread_t s5;

  trace().printf("Testing threaded floating point computations...\n");

  ASSERT_EQ(0, pthread_mutex_init(&recursion_depth_lock, 0));

  ASSERT_EQ(0, pthread_create(&s5, 0, stir, stirseed));
  recurse();
  trace().printf("Exiting from main.\n");
}
