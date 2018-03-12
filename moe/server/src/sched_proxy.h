/*
 * (c) 2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/hlist>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/libkproxy/scheduler_svr>

#include "globals.h"
#include "server_obj.h"

class Sched_proxy :
  public L4::Epiface_t<Sched_proxy, L4::Scheduler, Moe::Server_object>,
  public L4kproxy::Scheduler_svr_t<Sched_proxy>,
  public L4Re::Util::Icu_cap_array_svr<Sched_proxy>,
  public cxx::H_list_item_t<Sched_proxy>
{
  typedef L4Re::Util::Icu_cap_array_svr<Sched_proxy> Icu;

public:
  using L4Re::Util::Icu_cap_array_svr<Sched_proxy>::op_info;
  using L4kproxy::Scheduler_svr_t<Sched_proxy>::op_info;

  typedef L4::Cap<L4::Irq> Irq_cap;
  static Irq_cap alloc_irq_cap()
  { return object_pool.cap_alloc()->alloc<L4::Irq>(); }

  static void free_irq_cap(Irq_cap cap)
  { object_pool.cap_alloc()->free(cap); }

  Sched_proxy();
  ~Sched_proxy();

  int info(l4_umword_t *cpu_max, l4_sched_cpu_set_t *cpus);

  int run_thread(L4::Cap<L4::Thread> thread, l4_sched_param_t const &sp);

  int idle_time(l4_sched_cpu_set_t const &cpus, l4_kernel_clock_t &us);

  void set_prio(unsigned offs, unsigned limit)
  { _prio_offset = offs; _prio_limit = limit; }

  L4::Cap<L4::Thread> received_thread(L4::Ipc::Snd_fpage const &fp);
  L4::Cap<void> rcv_cap() const
  { return L4::Cap<L4::Thread>(Rcv_cap << L4_CAP_SHIFT); }

  void restrict_cpus(l4_umword_t cpus);
  void rescan_cpus();

  Icu::Irq *scheduler_irq() { return &_scheduler_irq; }
  Icu::Irq const *scheduler_irq() const { return &_scheduler_irq; }

private:
  friend class Cpu_hotplug_server;

  l4_sched_cpu_set_t _cpus, _real_cpus, _cpu_mask;
  unsigned _max_cpus;
  unsigned _prio_offset, _prio_limit;
  Icu::Irq _scheduler_irq;

  typedef cxx::H_list_t_bss<Sched_proxy> List;
  static List _list;
};

