/*
 * (c) 2008-2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/re/log>
#include <l4/re/env>
#include <l4/re/rm>
#include <l4/sys/scheduler>
#include <l4/re/util/object_registry>
#include <l4/re/util/cap_alloc>

#include <l4/sys/cxx/ipc_epiface>
#include "server.h"

class App_task :
  public L4::Epiface_t<App_task, L4Re::Parent, Ned::Server_object>
{
private:
  long _ref_cnt;

public:
  enum State { Initializing, Running, Zombie };

  long remove_ref() { return --_ref_cnt; }
  void add_ref() { ++_ref_cnt; }

  long ref_cnt() const { return _ref_cnt; }

private:

  template<typename T> using Unique_del_cap = L4Re::Util::Unique_del_cap<T>;
  template<typename T> using Unique_cap = L4Re::Util::Unique_cap<T>;

  Ned::Registry *_r;

  Unique_del_cap<L4::Task> _task;
  Unique_del_cap<L4::Thread> _thread;
  Unique_del_cap<L4Re::Rm> _rm;

  State _state;
  unsigned long _exit_code;
  l4_cap_idx_t _observer;

public:
  State state() const { return _state; }
  unsigned long exit_code() const { return _exit_code; }
  l4_cap_idx_t observer() const { return _observer; }
  void observer(l4_cap_idx_t o) { _observer = o; }
  void running()
  {
    _state = Running;
    add_ref();
  }


  App_task(Ned::Registry *r, L4::Cap<L4::Factory> alloc);

  //L4::Cap<L4Re::Mem_alloc> allocator() const { return _ma; }

  int op_signal(L4Re::Parent::Rights, unsigned long, unsigned long);

  L4::Cap<L4Re::Rm> rm() { return _rm.get(); }
  L4::Cap<L4::Task> task_cap() const { return _task.get(); }
  L4::Cap<L4::Thread> thread_cap() const { return _thread.get(); }

  virtual void terminate();

  virtual ~App_task();
};
