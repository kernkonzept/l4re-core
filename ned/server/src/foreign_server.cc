/* SPDX-License-Identifier: GPL-2.0-only OR License-Ref-kk-custom */
/*
 * Copyright (C) 2023 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 */

#include <pthread-l4.h>

#include <l4/re/error_helper>
#include <l4/sys/debugger.h>

#include "debug.h"
#include "foreign_server.h"

namespace Ned {

/**
 * Special registry for a foreign thread.
 *
 * Every register/unregister operation is synchronized with the foreign thread.
 */
class Foreign_registry : public L4Re::Util::Object_registry
{
  Foreign_server *_server;

  struct Pause_dispatch
  {
    Foreign_server *s;

    explicit Pause_dispatch(Foreign_registry *r)
    : s(r->_server)
    { s->pause_dispatch(); }

    ~Pause_dispatch() { s->resume_dispatch(); }
  };

public:
  Foreign_registry(Foreign_server *server, L4::Cap<L4::Thread> t,
                   L4::Cap<L4::Factory> f)
  : L4Re::Util::Object_registry(server, t, f), _server(server)
  {}

  L4::Cap<void>
  register_obj(L4::Epiface *o, char const *service) override
  {
    Pause_dispatch pause(this);
    return L4Re::Util::Object_registry::register_obj(o, service);
  }

  L4::Cap<void>
  register_obj(L4::Epiface *o) override
  {
    Pause_dispatch pause(this);
    return L4Re::Util::Object_registry::register_obj(o);
  }

  L4::Cap<L4::Irq>
  register_irq_obj(L4::Epiface *o) override
  {
    Pause_dispatch pause(this);
    return L4Re::Util::Object_registry::register_irq_obj(o);
  }

  L4::Cap<L4::Rcv_endpoint>
  register_obj(L4::Epiface *o, L4::Cap<L4::Rcv_endpoint> ep) override
  {
    Pause_dispatch pause(this);
    return L4Re::Util::Object_registry::register_obj(o, ep);
  }

  void
  unregister_obj(L4::Epiface *o, bool unmap) override
  {
    Pause_dispatch pause(this);
    L4Re::Util::Object_registry::unregister_obj(o, unmap);
  }
};

Foreign_server::Foreign_server()
{
  auto factory = L4Re::Env::env()->factory();
  _interrupt = L4Re::Util::make_unique_cap<L4::Irq>();
  L4Re::chksys(factory->create(_interrupt.get()));
  _ack = L4Re::Util::make_unique_cap<L4::Semaphore>();
  L4Re::chksys(factory->create(_ack.get()));
  _resume = L4Re::Util::make_unique_cap<L4::Semaphore>();
  L4Re::chksys(factory->create(_resume.get()));

  pthread_mutex_init(&_start_mutex, NULL);
  pthread_mutex_lock(&_start_mutex);

  pthread_attr_t attr;
  struct sched_param sp;

  pthread_attr_init(&attr);
  sp.sched_priority = 0xf1;
  pthread_attr_setschedpolicy(&attr, SCHED_L4);
  pthread_attr_setschedparam(&attr, &sp);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setstacksize(&attr, 0x1000);

  int r = pthread_create(&_th, &attr, &__run, this);
  if (r)
    Err().printf("error: could not start server thread: %d\n", r);

  l4_debugger_set_object_name(pthread_l4_cap(_th), "ned-svr");

  pthread_attr_destroy(&attr);
  pthread_mutex_lock(&_start_mutex);
  pthread_mutex_unlock(&_start_mutex);
  pthread_mutex_destroy(&_start_mutex);
}

Foreign_server::~Foreign_server() = default;

void *
Foreign_server::__run(void *a)
{
  reinterpret_cast<Foreign_server*>(a)->run();
  return a;
}

void
Foreign_server::run()
{
  _r =
    cxx::make_unique<Foreign_registry>(this, Pthread::L4::cap(pthread_self()),
                                       L4Re::Env::env()->factory());

  // Call explicitly base class to prevent deadlock
  _r->L4Re::Util::Object_registry::register_obj(this, _interrupt.get());

  pthread_mutex_unlock(&_start_mutex);
  loop_noexc(_r.get());
}

L4Re::Util::Object_registry *
Foreign_server::registry() const
{
  return _r.get();
}

void
Foreign_server::pause_dispatch()
{
  _interrupt->trigger();
  _ack->down();
}

void
Foreign_server::resume_dispatch()
{
  _resume->up();
}

void
Foreign_server::handle_irq()
{
  _ack->up();
  _resume->down();
}

}
