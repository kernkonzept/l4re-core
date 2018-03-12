/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Matthias Lange <matthias.lange@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Test the Scheduler API
 *
 * All tests assume that CPUs do not go to sleep during individual test runs.
 */

#include <l4/sys/scheduler>
#include <l4/re/error_helper>
#include <l4/re/env>

#include <pthread-l4.h>

#include <l4/atkins/tap/main>
#include <l4/atkins/debug>
#include <l4/atkins/l4_assert>


static Atkins::Dbg dbg{2};

TEST(Scheduler, SchedulerInfo)
{
  l4_sched_cpu_set_t cs = l4_sched_cpu_set(0, 0);
  l4_umword_t max_cpus = 0;
  EXPECT_L4OK(L4Re::Env::env()->scheduler()->info(&max_cpus, &cs));
  // request scheduler infos of CPUs beyond max_cpus
  cs = l4_sched_cpu_set(max_cpus + 1, 0);
  EXPECT_L4ERR(L4_ERANGE, L4Re::Env::env()->scheduler()->info(&max_cpus, &cs));
}

/**
 * Test Scheduler::is_online
 *
 * Scheduler::is_online uses Scheduler::info internally. That's why this test
 * first gathers the scheduler info. Then Scheduler::is_online should return
 * true for all CPUs whose bit is set in cs.map after the initial
 * Scheduler::info call. We don't test the result of Scheduler::is_online of
 * not set CPUs as they might come online during the test. However,
 * Scheduler::is_online must return false if a CPU number beyond max_cpus is
 * tested.
 */
TEST(Scheduler, SchedulerIsOnline)
{
  l4_sched_cpu_set_t cs = l4_sched_cpu_set(0, 0);
  l4_umword_t max_cpus = 0;
  EXPECT_L4OK(L4Re::Env::env()->scheduler()->info(&max_cpus, &cs));

  // all CPUs marked in cs.map should be online
  for (unsigned i = 0; i < max_cpus; ++i)
      if (cs.map & (1UL << i))
        EXPECT_TRUE(L4Re::Env::env()->scheduler()->is_online(i));

  // CPUs beyond max_cpus should never be online
  EXPECT_FALSE(L4Re::Env::env()->scheduler()->is_online(max_cpus));
}

/**
 * Test Scheduler::idle_time
 *
 * The test queries the idle time of all CPUs marked online in cs.map. The test
 * does not test the correctness of the returned idle time as the correct idle
 * time of a CPU can only be queried on the CPU itself, but the test does not
 * create individual threads for each CPU.
 */
TEST(Scheduler, SchedulerIdleTime)
{
  l4_sched_cpu_set_t cs = l4_sched_cpu_set(0, 0);
  l4_umword_t max_cpus = 0;
  EXPECT_L4OK(L4Re::Env::env()->scheduler()->info(&max_cpus, &cs));
  l4_umword_t online = cs.map;

  // skip the rest of the test if idle_time() is not implemented by the
  // scheduler
  l4_kernel_clock_t us;
  long err = l4_error(L4Re::Env::env()->scheduler()->idle_time(cs, &us));
  if (err == -L4_ENOSYS)
    {
      RecordProperty("SKIP", "The scheduler does not implement idle_time()");
      return;
    }

  // query idle time from all online CPUs
  for (unsigned i = 0; i < max_cpus; ++i)
    {
      cs.map = (1UL << i);
      if (online & cs.map)
        EXPECT_L4OK(L4Re::Env::env()->scheduler()->idle_time(cs, &us));
    }

  // Scheduler::idle_time of CPU beyond max_cpus must return -L4_EINVAL
  cs.gran_offset = max_cpus;
  cs.map = 1;
  EXPECT_L4ERR(L4_EINVAL, L4Re::Env::env()->scheduler()->idle_time(cs, &us));
}

static void *thread_fn(void *)
{
  while (1)
    sleep(1);

  return 0;
}

TEST(Scheduler, SchedulerRunThread)
{
  pthread_t t;

  pthread_create(&t, NULL, thread_fn, NULL);
  L4::Cap<L4::Thread> cap = L4::Cap<L4::Thread>(pthread_l4_cap(t));

  l4_sched_cpu_set_t cs = l4_sched_cpu_set(0, 0);
  l4_umword_t max_cpus = 0;
  EXPECT_L4OK(L4Re::Env::env()->scheduler()->info(&max_cpus, &cs));

  // find online CPU
  unsigned c = 0;
  while (!(cs.map & (1UL << c)) && (c < sizeof(l4_umword_t) * 8))
    c++;

  // start thread on an online CPU
  l4_sched_param_t sp = l4_sched_param(20);
  sp.affinity = l4_sched_cpu_set(c, 0);
  EXPECT_L4OK(L4Re::Env::env()->scheduler()->run_thread(cap, sp));

  // find offline CPU
  c = 0;
  while ((cs.map & (1UL << c)) && (c < sizeof(l4_umword_t) * 8))
    c++;
  // launching a thread on an offline CPU is ok, but it may never run
  // see documentation of Scheduler::run_thread
  if (!L4Re::Env::env()->scheduler()->is_online(c))
    {
      sp.affinity = l4_sched_cpu_set(c, 0);
      EXPECT_L4OK(L4Re::Env::env()->scheduler()->run_thread(cap, sp));
    }
}
