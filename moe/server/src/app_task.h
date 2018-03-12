/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "dataspace.h"
#include "log.h"
#include "alloc.h"
#include "region.h"

#include <l4/sys/cxx/ipc_epiface>
#include <l4/sys/capability>
#include <l4/re/parent>

#include <cstring>

class App_task : public L4::Epiface_t<App_task, L4Re::Parent, Moe::Server_object>
{

private:
  L4::Cap<L4::Task> _task;
  L4::Cap<L4::Thread> _thread;


  Allocator *_alloc;
  cxx::Weak_ref<Region_map> _rm;

public:
  App_task();
  Allocator *allocator() const { return _alloc; }
  void set_allocator(Allocator *a) { _alloc = a; }

  cxx::Weak_ref<Region_map> const &rm() { return _rm; }
  long op_signal(L4Re::Parent::Rights, unsigned long, unsigned long);

  void task_cap(L4::Cap<L4::Task> const &c) { _task = c; }
  void thread_cap(L4::Cap<L4::Thread> const &c) { _thread = c; }

  L4::Cap<L4::Task> task_cap() const { return _task; }
  L4::Cap<L4::Thread> thread_cap() const { return _thread; }

  virtual ~App_task();
};
