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
#include <l4/util/util.h>

#include <pthread-l4.h>
#include <thread>
#include <thread-l4>
#include <vector>

#include <l4/atkins/tap/main>
#include <l4/atkins/debug>
#include <l4/atkins/l4_assert>
#include <l4/atkins/ipc_helper>


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

static void ping_thread(L4::Cap<L4::Thread> from, long terminate)
{
  float cnt = 0;
  while (1)
    {
      // Do some time consuming counting before waiting.
      cnt = 0;
      for(; cnt < 30000; cnt+= 0.3)
        cnt -= 0.1;

      long rec = l4_error(l4_ipc_receive(from.cap(), l4_utcb(), L4_IPC_NEVER));

      l4_ipc_send(L4_INVALID_CAP | L4_SYSF_REPLY, l4_utcb(),
                  l4_msgtag(rec, 0, 0, 0), L4_IPC_NEVER);
      if (rec == terminate)
        break;
    }
}

/**
 * Migrates a thread to all online (max: sizeof(cs.map)*8) cores, sends an IPC
 * and checks that it actually consumed time there via stats_time.
 *
 * \pre The kernel does not change the scheduling parameters.
 * \pre The kernel schedules a thread on the first available CPU in the cpu set
 *      map.
 */
TEST(Scheduler, SchedulerMigrateThread)
{
  using Atkins::Ipc_helper::ipc_call;

  long terminate = 250;
  auto t1 =
    std::thread(ping_thread, L4Re::Env::env()->main_thread(), terminate);
  L4::Cap<L4::Thread> cap1 = std::L4::thread_cap(t1);

  l4_sched_cpu_set_t cs = l4_sched_cpu_set(0, 0);
  l4_umword_t max_cpus = 0;
  L4Re::chksys(L4Re::Env::env()->scheduler()->info(&max_cpus, &cs),
               "Requesting scheduling information.");

  l4_umword_t current_shift = 0;
  l4_kernel_clock_t current = 0;
  long tag_label = 170;

  l4_umword_t max_cpu_shift =
    (sizeof(cs.map) * 8) < max_cpus ? sizeof(cs.map) * 8 : max_cpus;

  // find first online
  while (!(cs.map & (1UL << current_shift)) && (current_shift < max_cpu_shift))
    ++current_shift;

  long ret;
  while (current_shift < max_cpu_shift)
    {
      l4_sched_param_t sp = l4_sched_param(2);
      sp.affinity = l4_sched_cpu_set(current_shift, 0);
      EXPECT_L4OK(L4Re::Env::env()->scheduler()->run_thread(cap1, sp))
        << "Update the scheduling parameters for a thread.";

      if (tag_label != (ret = ipc_call(cap1, tag_label)))
        dbg.printf(
          "reply did not contain expected tag_label %li; received: %li\n",
          tag_label, ret);

      l4_kernel_clock_t us = 0;
      EXPECT_L4OK(cap1->stats_time(&us)) << "Requesting thread statistics.";
      EXPECT_LT(current, us) << "The thread consumed time on core "
                             << current_shift << ".";
      current = us;

      ++current_shift; // increase counter and search for the next online one
      while (!(cs.map & (1UL << current_shift))
             && (current_shift < max_cpu_shift))
        ++current_shift;
    }

  if (terminate != (ret = ipc_call(cap1, terminate)))
    dbg.printf(
      "reply did not contain expected termiante label %li; received: %li\n",
      terminate, ret);

  t1.join();
}

