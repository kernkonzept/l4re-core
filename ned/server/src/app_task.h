/*
 * (c) 2008-2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/ref_ptr>
#include <l4/re/log>
#include <l4/re/env>
#include <l4/re/rm>
#include <l4/sys/scheduler>
#include <l4/re/util/object_registry>
#include <l4/re/util/cap_alloc>

#include <l4/sys/cxx/ipc_epiface>
#include "foreign_server.h"

class App_task :
  public L4::Irqep_t<App_task>
{
private:
  long _ref_cnt;

  /**
   * Helper to receive the task exit signal in a separate thread.
   */
  struct Parent_receiver :
    public L4::Epiface_t<Parent_receiver, L4Re::Parent,
                         Ned::Foreign_server_object>
  {
    Parent_receiver(App_task &parent);

    int op_signal(L4Re::Parent::Rights, unsigned long, unsigned long);

    App_task &parent;
    unsigned long exit_code;
    L4Re::Util::Unique_cap<L4::Semaphore> wait;
  };

public:
  enum State { Initializing, Running, Zombie };

  long remove_ref() { return --_ref_cnt; }
  void add_ref() { ++_ref_cnt; }

  long ref_cnt() const { return _ref_cnt; }

private:

  template<typename T> using Unique_del_cap = L4Re::Util::Unique_del_cap<T>;
  template<typename T> using Unique_cap = L4Re::Util::Unique_cap<T>;

  Unique_del_cap<L4::Task> _task;
  Unique_del_cap<L4::Thread> _thread;
  Unique_del_cap<L4Re::Rm> _rm;

  State _state;
  unsigned long _exit_code;
  bool _exit_code_valid;

  Parent_receiver _parent_receiver;

  /// `Ref_ptr` to `this` or `nullptr`.
  /// Used to keep reference count up for preventing deletion of instance while
  /// `L4Re::Parent` still needs to be served.
  cxx::Ref_ptr<App_task> _self;

  void reset();

protected:
  virtual void dispatch_exit_signal();

public:
  State state() const { return _state; }
  unsigned long exit_code() const { return _exit_code; }
  bool exit_code_valid() const { return _exit_code_valid; }

  // Instance is passed via `Ref_ptr` to hamper accidental usage of this
  // function on stack allocated objects.
  static void running(cxx::Ref_ptr<App_task> self)
  {
    self->_state = Running;
    self->_self = self;
  }

  App_task(L4Re::Util::Ref_cap<L4::Factory>::Cap const &alloc);
  virtual ~App_task();

  void handle_irq();

  L4::Cap<L4Re::Rm> rm() { return _rm.get(); }
  L4::Cap<L4::Task> task_cap() const { return _task.get(); }
  L4::Cap<L4::Thread> thread_cap() const { return _thread.get(); }
  L4::Cap<L4Re::Parent> parent_cap() const { return _parent_receiver.obj_cap(); }

  void terminate();
  void wait();
};
