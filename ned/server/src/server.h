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
#include <l4/re/util/object_registry>
#include <pthread.h>
#include <cstdio>

namespace Ned {

class Server_object : public L4::Epiface
{};

class Registry : public L4Re::Util::Object_registry
{
public:
  Registry(L4::Ipc_svr::Server_iface *sif,
           L4::Cap<L4::Thread> t, L4::Cap<L4::Factory> f)
  : L4Re::Util::Object_registry(sif, t, f) {}

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


