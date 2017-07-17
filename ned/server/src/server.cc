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

#include <l4/re/error_helper>
#include <l4/re/env>

namespace Ned {

using L4Re::chksys;

Server_object::List::List() : _f(0)
{
  pthread_mutex_init(&_lock, NULL);
}

Server_object::List::~List()
{
  pthread_mutex_lock(&_lock);
  for (Server_object *o = _f; o;)
    {
      Server_object *n = o->_n;
      assert(o->_l == this);

      o->_l = 0;
      o->_p = 0;
      o->_n = 0;
      o = n;
    }
  pthread_mutex_unlock(&_lock);

  pthread_mutex_destroy(&_lock);
}


void Server_object::List::add(Server_object *o)
{
  if (o->_l) return;
  pthread_mutex_lock(&_lock);
  o->_l = this;
  o->_n = _f;
  o->_p = 0;
  if (_f)
    _f->_p = o;

  _f = o;
  pthread_mutex_unlock(&_lock);
}

Server_object *Server_object::List::remove(Server_object *o)
{
  Server_object *res = 0;
  pthread_mutex_lock(&_lock);
  if (!o) o = _f;
  if (o && o->_l == this)
    {
      if (o->_p)
	o->_p->_n = o->_n;
      else
	_f = o->_n;

      if (o->_n)
	o->_n->_p = o->_p;

      o->_l = 0;
      o->_n = 0;
      o->_p = 0;
      res = o;
    }
  pthread_mutex_unlock(&_lock);
  return res;
}

Server::Server() : Base(0)
{
  pthread_mutex_init(&_start_mutex, NULL);
  pthread_mutex_lock(&_start_mutex);
  chksys(pthread_create(&_th, NULL, &__run, this), "creating server thread");
  pthread_mutex_lock(&_start_mutex);
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
