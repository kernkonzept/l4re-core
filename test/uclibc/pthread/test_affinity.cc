/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Benjamin Lamowski <benjamin.lamowski@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/*
 * \file
 * Set and get a pthread's CPU Affinity.
 */

#include <pthread.h>

#include <l4/atkins/tap/main>

/**
 * Get the number of available CPUs.
 */
static l4_size_t hw_cpu_count()
{
  char *cpus = getenv("NUM_CPUS");
  if (cpus)
    return atol(cpus);
  return 1;
}

/**
 * After setting the CPU affinity of the current thread, getting its affinity
 * returns the selected CPU.
 */
TEST(Pthread, Affinity)
{
  auto max_cpus = hw_cpu_count();
  int const cpu_no = max_cpus - 1;

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_no, &cpuset);

  auto thread = pthread_self();
  ASSERT_EQ(0,  pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset))
      << "Set the CPU affinity of the current thread.";
  ASSERT_EQ(0,  pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset))
      << "Get the CPU affinity of the current thread.";
  ASSERT_TRUE(CPU_ISSET(cpu_no, &cpuset))
      << "The current thread's affinity is set correctly.";
}
