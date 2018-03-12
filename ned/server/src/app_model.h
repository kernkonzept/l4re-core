/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once


#include <l4/re/l4aux.h>

#include <l4/libloader/loader>
#include <l4/libloader/remote_app_model>
#include "remote_mem.h"
#include "app_task.h"
#include "debug.h"

#if 0
namespace Ldr {
class Script;
};
#endif

struct App_model : public Ldr::Base_app_model<Stack>
{
  enum
  {
    Task_cap               = 1,
    Factory_cap            = 2,
    Rm_thread_cap          = 3,
    Log_cap                = 5,
    External_rm_cap        = 6,
    Allocator_cap          = 7,
    Names_cap,
    Parent_cap,
    Kip_cap,
    Scheduler_cap,
    First_free,
  };

  enum Prios
  {
    Default_base_prio = 0x00,
    Default_max_prio  = 0xff,
  };

  enum
  {
#ifdef ARCH_mips
    Utcb_area_start        = 0x73000000, // this needs to be lower on MIPS
#else
    Utcb_area_start        = 0xb3000000,
#endif
  };

  typedef L4Re::Util::Ref_cap<L4Re::Dataspace>::Cap Const_dataspace;
  typedef L4Re::Util::Ref_cap<L4Re::Dataspace>::Cap Dataspace;
  typedef L4Re::Util::Ref_cap<L4Re::Rm>::Cap Rm;
#if 0
  typedef L4::Cap<L4Re::Dataspace> Const_dataspace;
  typedef L4::Cap<L4Re::Dataspace> Dataspace;
  typedef L4::Cap<L4Re::Rm> Rm;
#endif
  App_task *_task;

  explicit App_model();

  Dataspace alloc_ds(unsigned long size) const;

  static Const_dataspace open_file(char const *name);

  virtual l4_cap_idx_t push_initial_caps(l4_cap_idx_t start) = 0;
  virtual void map_initial_caps(L4::Cap<L4::Task> task, l4_cap_idx_t start) = 0;

  void prog_attach_ds(l4_addr_t addr, unsigned long size,
                      Const_dataspace ds, unsigned long offset,
                      unsigned flags, char const *what);

  static void copy_ds(Dataspace dst, unsigned long dst_offs,
                      Const_dataspace src, unsigned long src_offs,
                      unsigned long size);

  static bool all_segs_cow() { return false; }

  l4_addr_t local_attach_ds(Const_dataspace ds, unsigned long size,
                            unsigned long offset) const;

  void local_detach_ds(l4_addr_t addr, unsigned long size) const;

  int prog_reserve_area(l4_addr_t *start, unsigned long size, unsigned flags, unsigned char align);

  Dataspace alloc_app_stack();

  void init_prog();

  static Const_dataspace reserved_area()
  { return Const_dataspace(); }

  static Dataspace local_kip_ds()
  {
    extern l4re_aux_t* l4re_aux;
    return L4::Cap<L4Re::Dataspace>(l4re_aux->kip_ds);
  }


  static L4::Cap<void> local_kip_cap()
  { return local_kip_ds().get(); }
#if 0
  static L4::Cap<void> prog_kip_ds()
  { return L4::Cap<void>(Kip_cap << L4_CAP_SHIFT); }
#endif

  void get_task_caps(L4::Cap<L4::Factory> *factory,
                     L4::Cap<L4::Task> *task,
                     L4::Cap<L4::Thread> *thread);

  l4_msgtag_t run_thread(L4::Cap<L4::Thread> thread,
                         l4_sched_param_t const &sp)
  {
    L4::Cap<L4::Scheduler> s(prog_info()->scheduler.raw & (~0UL << L4_FPAGE_ADDR_SHIFT));

    // test whether intersection between provided cpu set and online cpu set
    // is empty, in that case warn that the thread _may_ never run
    l4_umword_t cpu_max;
    l4_sched_cpu_set_t cpus = sp.affinity;
    l4_msgtag_t t = s->info(&cpu_max, &cpus);
    if (l4_error(t))
      return t;

    if (!(cpus.map & sp.affinity.map))
      Dbg(Dbg::Warn).printf("warning: Launching thread on offline CPU. Thread may never run!\n");

    return s->run_thread(thread, sp);
  }

  virtual void push_argv_strings() = 0;
  virtual void push_env_strings() = 0;
#if 0
//private:
  L4::Cap<L4Re::Log> _log;
  L4::Cap<L4::Scheduler> _sched;
  L4::Cap<L4Re::Namespace> _ns;
  L4::Cap<L4Re::Mem_alloc> _ma;
  L4::Cap<L4Re::Mem_alloc> _ma;
  L4::Cap<L4::Factory> _factory;
#endif

  virtual ~App_model() throw() {}

};

typedef Ldr::Remote_app_model<App_model> Rmt_app_model;


