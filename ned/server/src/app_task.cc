/*
 * (c) 2008-2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "app_task.h"

#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>

#include "server.h"

using L4Re::Util::cap_alloc;
using L4Re::chkcap;
using L4Re::chksys;


#if 0
static Slab_alloc<App_task> *alloc()
{
  static Slab_alloc<App_task> a;
  return &a;
}
#endif

#if 0
void *App_task::operator new (size_t) noexcept
{ return alloc()->alloc(); }

void App_task::operator delete (void *m) noexcept
{ alloc()->free((App_task*)m); }
#endif

App_task::Parent_receiver::Parent_receiver(App_task &parent)
: parent(parent), exit_code(0)
{
  wait = L4Re::Util::make_unique_cap<L4::Semaphore>();
  chksys(L4Re::Env::env()->factory()->create(wait.get()),
         "Parent_receiver wait sem");
}

/**
 * Receiver of exit signals.
 *
 * This IPC handler is invoked in the context of the Foreign_server thread.
 * Store the exit information and wake up the main thread to dispatch the exit
 * signal handling there. See App_task::handle_irq().
 */
int
App_task::Parent_receiver::op_signal(L4Re::Parent::Rights, unsigned long sig,
                                     unsigned long val)
{
  switch (sig)
    {
    case 0: // exit
      exit_code = val;
      wait->up();
      parent.obj_cap()->trigger();
      return -L4_ENOREPLY;
    default: break;
    }
  return L4_EOK;
}

/**
 * Signal handler in main thread for task termination.
 *
 * Task exit signals are received in the context of the Foreign_server thread.
 * See App_task::Parent_receiver::op_signal(). They trigger the respective
 * L4::Irq of the App_task to dispatch the exit signal handling in the main
 * thread.
 */
void
App_task::handle_irq()
{
  if (_state == Running)
    {
      _state = Zombie;
      _exit_code = _parent_receiver.exit_code;
      _exit_code_valid = true;
      reset();
      dispatch_exit_signal();

      _self = nullptr;  // keep at end; might delete current instance
    }
}

App_task::App_task(L4Re::Util::Ref_cap<L4::Factory>::Cap const &alloc)
: _ref_cnt(0),
  _task(chkcap(cap_alloc.alloc<L4::Task>(), "allocating task cap")),
  _thread(chkcap(cap_alloc.alloc<L4::Thread>(), "allocating thread cap")),
  _rm(chkcap(cap_alloc.alloc<L4Re::Rm>(), "allocating region-map cap")),
  _state(Initializing), _exit_code_valid(false),
  _parent_receiver(*this)
{
  chksys(alloc->create(_rm.get()), "allocating new region map");

  chkcap(Ned::server.registry()->register_irq_obj(this),
         "register App_task irq");

  chkcap(Ned::foreign_server->registry()->register_obj(&_parent_receiver),
         "register App_task parent rcv");
}

App_task::~App_task()
{
  reset();
}

void
App_task::reset()
{
  _task.reset();
  _thread.reset();
  _rm.reset();

  Ned::foreign_server->registry()->unregister_obj(&_parent_receiver);
  Ned::server.registry()->unregister_obj(this);
}

void
App_task::dispatch_exit_signal()
{}

void
App_task::terminate()
{
  if (_state == Running)
    {
      _state = Zombie;
      reset();
      dispatch_exit_signal();
      _self = nullptr;  // keep at end; might delete current instance
    }
}

void
App_task::wait()
{
  if (_state == App_task::Running)
    {
      _parent_receiver.wait->down();
      handle_irq();
    }
}
