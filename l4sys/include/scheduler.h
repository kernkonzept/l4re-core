/**
 * \file
 * Scheduler object functions.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#include <l4/sys/kernel_object.h>
#include <l4/sys/ipc.h>

/**
 * \defgroup l4_scheduler_api Scheduler
 * \ingroup  l4_kernel_object_api
 * C interface of the Scheduler kernel object.
 *
 * The Scheduler interface allows a client to manage CPU resources. The API
 * provides functions to query scheduler information, check the online state
 * of CPUs, query CPU idle time and to start threads on defined CPU sets.
 *
 * \includefile{l4/sys/scheduler.h}
 */

/**
 * CPU sets.
 * \ingroup l4_scheduler_api
 */
typedef struct l4_sched_cpu_set_t
{
  /**
   * Combination of granularity and offset.
   *
   * The granularity defines how many CPUs each bit in map describes. And the
   * offset is the numer of the first CPU described by the first bit in the bitmap.
   * \pre offset must be a multiple of 2^granularity.
   *
   * | MSB              |                 LSB |
   * |:-----------------|--------------------:|
   * | 8bit granularity | 24bit offset ..     |
   */
  l4_umword_t gran_offset;

  /**
   * Bitmap of CPUs.
   */
  l4_umword_t map;

#ifdef __cplusplus
  /// \return Get granularity value
  unsigned char granularity() const { return gran_offset >> 24; }
  /// \return Get offset value
  unsigned offset() const { return gran_offset & 0x00ffffff; }
  /// Set offset and granularity
  void set(unsigned char granularity, unsigned offset)
  { gran_offset = ((l4_umword_t)granularity << 24) | (offset & 0x00ffffff); }
#endif
} l4_sched_cpu_set_t;

/**
 *
 * \ingroup l4_scheduler_api
 *
 * \param offset       Offset.
 * \param granularity  Granularitry in log2 notation.
 * \param map          Bitmap of CPUs, defaults to 1 in C++.
 *
 * \return CPU set.
 */
L4_INLINE l4_sched_cpu_set_t
l4_sched_cpu_set(l4_umword_t offset, unsigned char granularity,
                 l4_umword_t map L4_DEFAULT_PARAM(1)) L4_NOTHROW;

/**
 * \ingroup l4_scheduler_api
 * \copybrief L4::Scheduler::info
 *
 * \param         scheduler  Scheduler object.
 * \param[out]    cpu_max    Maximum number of CPUs ever available.
 * \param[in,out] cpus       \a cpus.offset is first CPU of interest.
 *                           \a cpus.granularity (see l4_sched_cpu_set_t).
 *                           \a cpus.map Bitmap of online CPUs.
 *
 * \retval 0           Success.
 * \retval -L4_EINVAL  The given CPU offset is larger than the maximum number
 *                     of CPUs.
 */
L4_INLINE l4_msgtag_t
l4_scheduler_info(l4_cap_idx_t scheduler, l4_umword_t *cpu_max,
                  l4_sched_cpu_set_t *cpus) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_scheduler_info_u(l4_cap_idx_t scheduler, l4_umword_t *cpu_max,
                    l4_sched_cpu_set_t *cpus, l4_utcb_t *utcb) L4_NOTHROW;


/**
 * Scheduler parameter set.
 * \ingroup l4_scheduler_api
 */
typedef struct l4_sched_param_t
{
  l4_sched_cpu_set_t affinity; ///< CPU affinity.
  l4_umword_t        prio;     ///< Priority for scheduling.
  l4_umword_t        quantum;  ///< Timeslice in micro seconds.
} l4_sched_param_t;

/**
 * Construct scheduler parameter.
 * \ingroup l4_scheduler_api
 */
L4_INLINE l4_sched_param_t
l4_sched_param(unsigned prio,
               l4_cpu_time_t quantum L4_DEFAULT_PARAM(0)) L4_NOTHROW;

/**
 * \ingroup l4_scheduler_api
 * \copybrief L4::Scheduler::run_thread
 *
 * \param scheduler  Scheduler object.
 * \copydetails L4::Scheduler::run_thread
 */
L4_INLINE l4_msgtag_t
l4_scheduler_run_thread(l4_cap_idx_t scheduler,
                        l4_cap_idx_t thread, l4_sched_param_t const *sp) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_scheduler_run_thread_u(l4_cap_idx_t scheduler, l4_cap_idx_t thread,
                          l4_sched_param_t const *sp, l4_utcb_t *utcb) L4_NOTHROW;

/**
 * \ingroup l4_scheduler_api
 * \copybrief L4::Scheduler::idle_time
 *
 * \param scheduler   Scheduler object.
 * \copydetails L4::Scheduler::idle_time
 */
L4_INLINE l4_msgtag_t
l4_scheduler_idle_time(l4_cap_idx_t scheduler, l4_sched_cpu_set_t const *cpus,
                       l4_kernel_clock_t *us) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE l4_msgtag_t
l4_scheduler_idle_time_u(l4_cap_idx_t scheduler, l4_sched_cpu_set_t const *cpus,
                         l4_kernel_clock_t *us, l4_utcb_t *utcb) L4_NOTHROW;



/**
 * \ingroup l4_scheduler_api
 * \copybrief L4::Scheduler::is_online
 *
 * \param scheduler  Scheduler object.
 * \param cpu        CPU number whose online status should be queried.
 *
 * \retval true   The CPU is online.
 * \retval false  The CPU is offline
 */
L4_INLINE int
l4_scheduler_is_online(l4_cap_idx_t scheduler, l4_umword_t cpu) L4_NOTHROW;

/**
 * \internal
 */
L4_INLINE int
l4_scheduler_is_online_u(l4_cap_idx_t scheduler, l4_umword_t cpu,
                         l4_utcb_t *utcb) L4_NOTHROW;



/**
 * Operations on the Scheduler object.
 * \ingroup l4_scheduler_api
 * \hideinitializer
 * \internal
 */
enum L4_scheduler_ops
{
  L4_SCHEDULER_INFO_OP       = 0UL, /**< Query infos about the scheduler */
  L4_SCHEDULER_RUN_THREAD_OP = 1UL, /**< Run a thread on this scheduler */
  L4_SCHEDULER_IDLE_TIME_OP  = 2UL, /**< Query idle time for the scheduler */
};

/*************** Implementations *******************/

L4_INLINE l4_sched_cpu_set_t
l4_sched_cpu_set(l4_umword_t offset, unsigned char granularity,
                 l4_umword_t map) L4_NOTHROW
{
  l4_sched_cpu_set_t cs;
  cs.gran_offset = ((l4_umword_t)granularity << 24) | offset;
  cs.map         = map;
  return cs;
}

L4_INLINE l4_sched_param_t
l4_sched_param(unsigned prio, l4_cpu_time_t quantum) L4_NOTHROW
{
  l4_sched_param_t sp;
  sp.prio     = prio;
  sp.quantum  = quantum;
  sp.affinity = l4_sched_cpu_set(0, ~0, 1);
  return sp;
}


L4_INLINE l4_msgtag_t
l4_scheduler_info_u(l4_cap_idx_t scheduler, l4_umword_t *cpu_max,
                    l4_sched_cpu_set_t *cpus, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  l4_msgtag_t res;

  m->mr[0] = L4_SCHEDULER_INFO_OP;
  m->mr[1] = cpus->gran_offset;

  res = l4_ipc_call(scheduler, utcb, l4_msgtag(L4_PROTO_SCHEDULER, 2, 0, 0), L4_IPC_NEVER);

  if (l4_msgtag_has_error(res))
    return res;

  cpus->map = m->mr[0];

  if (cpu_max)
    *cpu_max = m->mr[1];

  return res;
}

L4_INLINE l4_msgtag_t
l4_scheduler_run_thread_u(l4_cap_idx_t scheduler, l4_cap_idx_t thread,
                          l4_sched_param_t const *sp, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *m = l4_utcb_mr_u(utcb);
  m->mr[0] = L4_SCHEDULER_RUN_THREAD_OP;
  m->mr[1] = sp->affinity.gran_offset;
  m->mr[2] = sp->affinity.map;
  m->mr[3] = sp->prio;
  m->mr[4] = sp->quantum;
  m->mr[5] = l4_map_obj_control(0, 0);
  m->mr[6] = l4_obj_fpage(thread, 0, L4_FPAGE_RWX).raw;

  return l4_ipc_call(scheduler, utcb, l4_msgtag(L4_PROTO_SCHEDULER, 5, 1, 0), L4_IPC_NEVER);
}

L4_INLINE l4_msgtag_t
l4_scheduler_idle_time_u(l4_cap_idx_t scheduler, l4_sched_cpu_set_t const *cpus,
                         l4_kernel_clock_t *us, l4_utcb_t *utcb) L4_NOTHROW
{
  l4_msg_regs_t *v = l4_utcb_mr_u(utcb);
  l4_msgtag_t res;

  v->mr[0] = L4_SCHEDULER_IDLE_TIME_OP;
  v->mr[1] = cpus->gran_offset;
  v->mr[2] = cpus->map;

  res = l4_ipc_call(scheduler, utcb,
                    l4_msgtag(L4_PROTO_SCHEDULER, 3, 0, 0), L4_IPC_NEVER);

  if (l4_msgtag_has_error(res))
    return res;

  *us = v->mr64[l4_utcb_mr64_idx(0)];

  return res;
}


L4_INLINE int
l4_scheduler_is_online_u(l4_cap_idx_t scheduler, l4_umword_t cpu,
                         l4_utcb_t *utcb) L4_NOTHROW
{
  l4_sched_cpu_set_t s;
  l4_msgtag_t r;
  s.gran_offset = cpu;
  r = l4_scheduler_info_u(scheduler, NULL, &s, utcb);
  if (l4_msgtag_has_error(r) || l4_msgtag_label(r) < 0)
    return 0;

  return s.map & 1;
}


L4_INLINE l4_msgtag_t
l4_scheduler_info(l4_cap_idx_t scheduler, l4_umword_t *cpu_max,
                  l4_sched_cpu_set_t *cpus) L4_NOTHROW
{
  return l4_scheduler_info_u(scheduler, cpu_max, cpus, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_scheduler_run_thread(l4_cap_idx_t scheduler,
                        l4_cap_idx_t thread, l4_sched_param_t const *sp) L4_NOTHROW
{
  return l4_scheduler_run_thread_u(scheduler, thread, sp, l4_utcb());
}

L4_INLINE l4_msgtag_t
l4_scheduler_idle_time(l4_cap_idx_t scheduler, l4_sched_cpu_set_t const *cpus,
                       l4_kernel_clock_t *us) L4_NOTHROW
{
  return l4_scheduler_idle_time_u(scheduler, cpus, us, l4_utcb());
}

L4_INLINE int
l4_scheduler_is_online(l4_cap_idx_t scheduler, l4_umword_t cpu) L4_NOTHROW
{
  return l4_scheduler_is_online_u(scheduler, cpu, l4_utcb());
}
