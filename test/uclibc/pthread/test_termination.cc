/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test detaching and joining of pthreads.
 */

#include <l4/util/util.h>
#include <pthread.h>

#include <l4/atkins/tap/main>

static void *direct_exit_func(void *)
{
  return (void *) 0x1234L;
}

static void *sleepy_func(void *)
{
  l4_sleep(1000);

  return (void *) 0x9876L;
}

static void *detaching_func(void *)
{
  pthread_detach(pthread_self());

  return (void *) 0x4567L;
}


TEST(Pthread, DetachTerminated)
{
  pthread_t t;

  ASSERT_EQ(0, pthread_create(&t, 0, direct_exit_func, (void *) 0));
  l4_sleep(50);
  ASSERT_EQ(0, pthread_detach(t));
}

TEST(Pthread, JoinAfterExit)
{
  pthread_t t;
  void *ret;

  ASSERT_EQ(0, pthread_create(&t, 0, direct_exit_func, (void *) 0));
  l4_sleep(50);
  ASSERT_EQ(0, pthread_join(t, &ret));
  EXPECT_EQ(0x1234L, (long) ret);
}

TEST(Pthread, JoinBeforeExit)
{
  pthread_t t;
  void *ret;

  ASSERT_EQ(0, pthread_create(&t, 0, sleepy_func, (void *) 0));
  ASSERT_EQ(0, pthread_join(t, &ret));
  EXPECT_EQ(0x9876L, (long) ret);
}

TEST(Pthread, ExitDetached)
{
  pthread_t t;

  ASSERT_EQ(0, pthread_create(&t, 0, detaching_func, (void *) 0));
  l4_sleep(50);
}

// libpthread requires the stack base address to be aligned to
// a 32-byte boundary
enum { STACK_ALIGN = 32 };

static char thread_stack[20000 + STACK_ALIGN];

TEST(Pthread, LocalStack)
{
  l4_addr_t stack_base = (l4_addr_t)(thread_stack + STACK_ALIGN)
                         & ~(STACK_ALIGN - 1);

  pthread_attr_t attr;
  pthread_t      thread;
  void *ret = NULL;

  ASSERT_EQ(0, pthread_attr_init(&attr));
  ASSERT_EQ(0, pthread_attr_setstack(&attr, (char*)stack_base, 20000UL));
  ASSERT_EQ(0, pthread_create(&thread, &attr, direct_exit_func, NULL));
  ASSERT_EQ(0, pthread_join(thread, &ret));
  EXPECT_EQ(0x1234L, (long) ret);
}

