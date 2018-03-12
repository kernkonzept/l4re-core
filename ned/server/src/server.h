/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/exceptions>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/cxx/auto_ptr>
#include <l4/re/util/object_registry>
#include <pthread.h>
#include <cstdio>

namespace Ned {

class Server_object : public L4::Epiface
{
public:
  class List
  {
  private:
    pthread_mutex_t _lock;
    Server_object *_f;

  public:
    List();
    ~List();
    void add(Server_object *o);
    Server_object *remove(Server_object *o);
  };

  Server_object() : _p(0), _n(0), _l(0) {}

private:
  Server_object *_p, *_n;
  List *_l;

};

class Registry : public L4Re::Util::Object_registry
{
private:
  Server_object::List _reap_list;

public:
  Registry(L4::Ipc_svr::Server_iface *sif,
           L4::Cap<L4::Thread> t, L4::Cap<L4::Factory> f)
  : L4Re::Util::Object_registry(sif, t, f), _reap_list() {}
  Server_object::List *reap_list() { return &_reap_list; }

  L4::Cap<void> register_obj(Server_object *o)
  { return L4Re::Util::Object_registry::register_obj(o); }

  void unregister_obj(Server_object *o)
  {
    _reap_list.remove(o);
    L4Re::Util::Object_registry::unregister_obj(o);
  }

  ~Registry() { printf("destroy registry\n"); }
};

class Server : public L4::Server<>
{
private:
  Registry *_r;
  pthread_t _th;
  pthread_mutex_t _start_mutex;
  static void *__run(void *);

  void run();
public:
  typedef L4::Server<> Base;

  Server();

  Registry *registry() { return _r; }
};

extern Server *server;

}


