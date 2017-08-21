/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *            Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * Test mutexes.
 */

#include <l4/re/env.h>
#include <l4/util/util.h>
#include <pthread.h>
#include <pthread-l4.h>
#include <sys/time.h>
#include <cerrno>

#include <l4/atkins/tap/main>


static long _mtx_thread_cnt;
static long _mtx_thread_iter = 1L<<17;

static void mtx_loop(pthread_mutex_t *m)
{
  for (int i = 0; i < _mtx_thread_iter; ++i)
    {
      ASSERT_EQ(0, pthread_mutex_lock(m));
      ++_mtx_thread_cnt;
      ASSERT_EQ(0, pthread_mutex_unlock(m));
    }
}

static void *mtx_thread(void *arg)
{
  mtx_loop((pthread_mutex_t*)arg);

  return (void*)1;
}

/**
 * A pthread mutex is owned by exactly one of two concurrent threads at the
 * same point in time.
 */
TEST(Pthread, Mutex)
{
  pthread_t t1, t2;
  pthread_mutex_t mtx;

  _mtx_thread_cnt = 0;
  ASSERT_EQ(0, pthread_mutex_init(&mtx, NULL));

  ASSERT_EQ(0, pthread_create(&t1, NULL, mtx_thread, (void*)&mtx));
  ASSERT_EQ(0, pthread_create(&t2, NULL, mtx_thread, (void*)&mtx));
  ASSERT_EQ(0, pthread_join(t1, NULL));
  ASSERT_EQ(0, pthread_join(t2, NULL));

  // the 2 threads have incremented the counter _mtx_thread_iter times each
  ASSERT_EQ(_mtx_thread_cnt, 2 * _mtx_thread_iter);

  ASSERT_EQ(0, pthread_mutex_destroy(&mtx));
}

struct comm_struct_t
{
  pthread_mutex_t m[2];
  int do_cross;
};

static void mtx_cross_loop(struct comm_struct_t *c)
{
  for (int i = 0; i < _mtx_thread_iter; ++i)
  {
    ASSERT_EQ(0, pthread_mutex_unlock(&c->m[1]));
    ++_mtx_thread_cnt;
    ASSERT_EQ(0, pthread_mutex_lock(&c->m[0]));
  }
}

static void *mtx_comm_thread0(void *arg)
{
  mtx_cross_loop((struct comm_struct_t *) arg);

  return (void*)1;
}

static void test_mtx_comm_thread1(struct comm_struct_t *c)
{
  if (c->do_cross)
  {
    l4_sched_param_t sp = l4_sched_param(0xff, 0);
    sp.affinity = l4_sched_cpu_set(1, 0, 1);
    l4_scheduler_run_thread(l4re_env()->scheduler,
                            pthread_l4_cap(pthread_self()), &sp);
  }


  ASSERT_EQ(0, pthread_mutex_lock(&c->m[1]));
  while (1)
  {
    ASSERT_EQ(0, pthread_mutex_unlock(&c->m[0]));
    ++_mtx_thread_cnt;
    ASSERT_EQ(0, pthread_mutex_lock(&c->m[1]));
  }

}

static void *mtx_comm_thread1(void *arg)
{
  test_mtx_comm_thread1((struct comm_struct_t *)arg);

  return (void*)1;
}


class MutexCross : public ::testing::TestWithParam<bool> {};


TEST_P(MutexCross, DISABLED_Communication)
{
  bool do_cross = GetParam();
  if (do_cross) // XXX detect if we are on an SMP machine
    {
      RecordProperty("SKIP", "SMP support missing");
    }
  else
    {
      pthread_t t1, t2;
      struct comm_struct_t c;
      l4_cpu_time_t start, end, diff;

      c.do_cross = do_cross;

      _mtx_thread_cnt = 0;
      ASSERT_EQ(0, pthread_mutex_init(&c.m[0], NULL));
      ASSERT_EQ(0, pthread_mutex_init(&c.m[1], NULL));

      ASSERT_EQ(0, pthread_mutex_lock(&c.m[0]));
      ASSERT_EQ(0, pthread_mutex_lock(&c.m[1]));

      start = l4_kip_clock(l4re_kip());
      ASSERT_EQ(0, pthread_create(&t1, NULL, mtx_comm_thread0, (void*)&c));
      ASSERT_EQ(0, pthread_create(&t2, NULL, mtx_comm_thread1, (void*)&c));
      pthread_join(t1, NULL);
      end = l4_kip_clock(l4re_kip());
      pthread_cancel(t2);

      diff = end - start;
      fprintf(stderr, "%ld rounds, kclks: %lld => %lld us\n",
             _mtx_thread_iter, diff, diff / _mtx_thread_iter);

      pthread_mutex_destroy(&c.m[0]);
      pthread_mutex_destroy(&c.m[1]);
    }
}

static INSTANTIATE_TEST_CASE_P(CpuVariants, MutexCross, ::testing::Bool());

/**
 * A pthread mutex supports initialization, lock, unlock, and destruction.
 */
TEST(Pthread, MutexSimple)
{
  pthread_mutex_t m;

  ASSERT_EQ(0, pthread_mutex_init(&m, NULL));

  ASSERT_EQ(0, pthread_mutex_lock(&m));
  ASSERT_EQ(0, pthread_mutex_unlock(&m));
  ASSERT_EQ(0, pthread_mutex_destroy(&m));
}

/**
 * A statically initialized pthread mutex supports lock, unlock, and
 * destruction.
 */
TEST(Pthread, MutexSimpleStatic)
{
  pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

  ASSERT_EQ(0, pthread_mutex_lock(&m));
  ASSERT_EQ(0, pthread_mutex_unlock(&m));
  ASSERT_EQ(0, pthread_mutex_destroy(&m));
}

/**
 * A recursive pthread mutex counts the number of lock and unlock operations.
 */
TEST(Pthread, MutexRecursive)
{
  pthread_mutex_t m;
  pthread_mutexattr_t a;

  ASSERT_EQ(0, pthread_mutexattr_init(&a));
  ASSERT_EQ(0, pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE));
  ASSERT_EQ(0, pthread_mutex_init(&m, &a));
  ASSERT_EQ(0, pthread_mutex_lock(&m));
  ASSERT_EQ(0, pthread_mutex_lock(&m));
  ASSERT_EQ(0, pthread_mutex_lock(&m));
  ASSERT_EQ(0, pthread_mutex_unlock(&m));
  ASSERT_EQ(0, pthread_mutex_unlock(&m));
  ASSERT_EQ(0, pthread_mutex_unlock(&m));

  ASSERT_EQ(0, pthread_mutex_destroy(&m));
  ASSERT_EQ(0, pthread_mutexattr_destroy(&a));
}

/**
 * An error-checking mutex returns an error when it is already locked by the
 * thread calling the lock operation again. The same behavior applies to unlock.
 */
TEST(Pthread, MutexErrorcheck)
{
  pthread_mutex_t m;
  pthread_mutexattr_t a;

  ASSERT_EQ(0, pthread_mutexattr_init(&a));
  ASSERT_EQ(0, pthread_mutexattr_settype(&a, PTHREAD_MUTEX_ERRORCHECK));
  ASSERT_EQ(0, pthread_mutex_init(&m, &a));
  ASSERT_EQ(0, pthread_mutex_lock(&m));
  ASSERT_NE(0, pthread_mutex_lock(&m));
  ASSERT_EQ(0, pthread_mutex_unlock(&m));
  ASSERT_NE(0, pthread_mutex_unlock(&m));

  ASSERT_EQ(0, pthread_mutex_destroy(&m));
  ASSERT_EQ(0, pthread_mutexattr_destroy(&a));
}

pthread_cond_t _cv;
pthread_mutex_t _mtx;
int _cnt;

static void test_condvar_thread()
{
  ASSERT_EQ(0, pthread_mutex_lock(&_mtx));
  ASSERT_EQ(0, pthread_cond_wait(&_cv, &_mtx));

  ++_cnt;

  ASSERT_EQ(0, pthread_mutex_unlock(&_mtx));
}

static void *condvar_thread(void *)
{
  test_condvar_thread();

  return NULL;
}

static void _condvar_thread_timed()
{
  struct timeval now;
  struct timespec timeout;

  ASSERT_EQ(0, pthread_mutex_lock(&_mtx));

  gettimeofday(&now, NULL);
  timeout.tv_sec = now.tv_sec;
  timeout.tv_nsec = now.tv_usec * 1000 + 2000000;
  ASSERT_EQ(ETIMEDOUT, pthread_cond_timedwait(&_cv, &_mtx, &timeout));
  ++_cnt;

  ASSERT_EQ(0, pthread_mutex_unlock(&_mtx));
}

static void *condvar_thread_timed(void *)
{
  _condvar_thread_timed();
  return NULL;
}

/**
 * Conditional variables block thread execution until the condition signal is
 * received.
 */
TEST(Pthread, CondvarSig)
{
  pthread_t t1, t2;
  ASSERT_EQ(0, pthread_cond_init(&_cv, NULL));
  ASSERT_EQ(0, pthread_mutex_init(&_mtx, NULL));
  _cnt = 0;

  ASSERT_EQ(0, pthread_create(&t1, NULL, condvar_thread, NULL));
  ASSERT_EQ(0, pthread_create(&t2, NULL, condvar_thread, NULL));

  l4_sleep(200);
  ASSERT_EQ(0, _cnt);

  pthread_cond_signal(&_cv);
  pthread_cond_signal(&_cv);
  ASSERT_EQ(0, pthread_join(t1, NULL));
  ASSERT_EQ(0, pthread_join(t2,NULL));
  ASSERT_EQ(2, _cnt);

  ASSERT_EQ(0, pthread_mutex_destroy(&_mtx));
  ASSERT_EQ(0, pthread_cond_destroy(&_cv));
}

/**
 * A broadcast signal of a condition reaches all threads blocking on the
 * conditional variable.
 */
TEST(Pthread, CondvarBroad)
{
  pthread_t t1, t2;
  ASSERT_EQ(0, pthread_cond_init(&_cv, NULL));
  ASSERT_EQ(0, pthread_mutex_init(&_mtx, NULL));
  _cnt = 0;

  ASSERT_EQ(0, pthread_create(&t1, NULL, condvar_thread, NULL));
  ASSERT_EQ(0, pthread_create(&t2, NULL, condvar_thread, NULL));

  l4_sleep(200);
  ASSERT_EQ(0, _cnt);

  ASSERT_EQ(0, pthread_cond_broadcast(&_cv));
  l4_sleep(200);
  ASSERT_EQ(2, _cnt);

  ASSERT_EQ(0, pthread_join(t1, NULL));
  ASSERT_EQ(0, pthread_join(t2, NULL));

  ASSERT_EQ(0, pthread_mutex_destroy(&_mtx));
  ASSERT_EQ(0, pthread_cond_destroy(&_cv));
}

/**
 * A timed wait on a conditional variable is a wait for a specified maximum
 * time. If this time passes and the timed-wait receives the timeout error, no
 * change in the conditional variable happens.  Another thread waiting for the
 * conditional to change remains blocked.
 */
TEST(Pthread, CondvarTimed)
{
  pthread_t t1, t2;
  ASSERT_EQ(0, pthread_cond_init(&_cv, NULL));
  ASSERT_EQ(0, pthread_mutex_init(&_mtx, NULL));
  _cnt = 0;

  ASSERT_EQ(0, pthread_create(&t1, NULL, condvar_thread, NULL));
  ASSERT_EQ(0, pthread_create(&t2, NULL, condvar_thread_timed, NULL));

  l4_sleep(200);
  ASSERT_EQ(1, _cnt);

  pthread_cond_signal(&_cv);
  ASSERT_EQ(0, pthread_join(t1, NULL));
  ASSERT_EQ(2, _cnt);

  ASSERT_EQ(0, pthread_join(t2,NULL));
  ASSERT_EQ(2, _cnt);

  ASSERT_EQ(0, pthread_mutex_destroy(&_mtx));
  ASSERT_EQ(0, pthread_cond_destroy(&_cv));
}
