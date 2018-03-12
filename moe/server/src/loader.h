/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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
#include <l4/cxx/string>

struct Moe_app_model : public Ldr::Base_app_model<Moe::Stack>
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

  typedef Moe::Dataspace const *Const_dataspace;
  typedef Moe::Dataspace *Dataspace;
  typedef Region_map *Rm;

  App_task *_task;
  cxx::String _prog;
  cxx::String _args;

  Moe_app_model(App_task *t, cxx::String const &prog, cxx::String const &args);

  Dataspace alloc_ds(unsigned long size) const;

  static Const_dataspace open_file(char const *name);

  void prog_attach_ds(l4_addr_t addr, unsigned long size,
                      Const_dataspace ds, unsigned long offset,
                      unsigned flags, char const *what);

  l4_cap_idx_t push_initial_caps(l4_cap_idx_t s);
  void map_initial_caps(L4::Cap<L4::Task>, l4_cap_idx_t);

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
  { return 0; }

  static Dataspace local_kip_ds()
  {
    return kip_ds;
  }

  static L4::Cap<void> local_kip_cap()
  { return kip_ds->obj_cap(); }

  static L4::Cap<void> prog_kip_ds()
  { return L4::Cap<void>(Kip_cap << L4_CAP_SHIFT); }

  L4Re::Env *add_env();
  void get_task_caps(L4::Cap<L4::Factory> *factory,
                     L4::Cap<L4::Task> *task,
                     L4::Cap<L4::Thread> *thread);

  l4_msgtag_t run_thread(L4::Cap<L4::Thread> thread,
                         l4_sched_param_t const &sp)
  { return L4Re::Env::env()->scheduler()->run_thread(thread, sp); }
};

typedef Ldr::Remote_app_model<Moe_app_model> Moe_x_app_model;

class Loader
{
public:


  virtual bool check_file_type(Moe::Dataspace const *file) const = 0;
  bool start(cxx::String const &init_prog, cxx::String const &args);
  bool exec(cxx::String const &prog, cxx::String const &args);
  virtual bool launch(App_task *, cxx::String const &, cxx::String const &) = 0;
  virtual ~Loader() {}
};
