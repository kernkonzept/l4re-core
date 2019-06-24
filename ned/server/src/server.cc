/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "server.h"

#include <pthread.h>
#include <pthread-l4.h>
#include <cassert>
#include <cstdio>

#include <l4/sys/debugger.h>
#include <l4/re/error_helper>
#include <l4/re/env>

namespace Ned {

using L4Re::chksys;

Server::Server() : Base(0)
{
  pthread_mutex_init(&_start_mutex, NULL);
  pthread_mutex_lock(&_start_mutex);

  pthread_attr_t attr;
  struct sched_param sp;

  pthread_attr_init(&attr);
  sp.sched_priority = 0xf1;
  pthread_attr_setschedpolicy(&attr, SCHED_L4);
  pthread_attr_setschedparam(&attr, &sp);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

  int r = pthread_create(&_th, &attr, &__run, this);
  if (r)
    fprintf(stderr, "error: could not start server thread: %d\n", r);

  l4_debugger_set_object_name(pthread_l4_cap(_th), "ned-svr");

  pthread_attr_destroy(&attr);
  pthread_mutex_lock(&_start_mutex);
  pthread_mutex_unlock(&_start_mutex);
  pthread_mutex_destroy(&_start_mutex);
}
void
Server::run()
{
  _utcb = l4_utcb();
  _r = new Registry(this, Pthread::L4::cap(pthread_self()),
                    L4Re::Env::env()->factory());
  pthread_mutex_unlock(&_start_mutex);
  loop<L4::Runtime_error>(_r);
}

void *
Server::__run(void *a)
{
  reinterpret_cast<Server*>(a)->run();
  return a;
}



}
