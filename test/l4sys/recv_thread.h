// vi:ft=cpp
/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#pragma once

#include <l4/sys/cxx/ipc_server_loop>
#include <l4/sys/scheduler>

#include <l4/re/error_helper>
#include <l4/re/util/cap_alloc>

#include <l4/atkins/factory>

#include <gtest/gtest.h>

#include <pthread-l4.h>
#include <pthread.h>

#include <array>

namespace {

constexpr int PING_VALUE = 0xafe;
constexpr int EXIT_LOOP = 0xbadc;
constexpr l4_umword_t DEFAULT_LABEL = 0;

template <int NUM_THREADS>
class Recv_thread
{
public:
  Recv_thread()
  {
    _running.fill(false);
    _irq = Atkins::Factory::del_cap<L4::Irq>();
    L4Re::Env::env()->factory()->create_irq(_irq.get());
  }

  ~Recv_thread()
  {
    for (unsigned i = 0; i < NUM_THREADS; ++i)
      terminate_thread(i);
  }

  L4::Cap<L4::Thread> thread_cap(unsigned i)
  {
    if (i > NUM_THREADS)
      throw L4::Runtime_error(L4_ERANGE);

    if (!_running[i])
      start_thread(i);

    auto ret = Pthread::L4::cap(_pthreads[i]);
    return ret;
  }

  void set_expected_label(l4_umword_t l, unsigned thread_num)
  {
    _disp[thread_num].label_expectation = l;
  }

  void terminate_thread(unsigned i)
  {
    if (!_running[i])
      return;

    L4Re::chksys(_irq->attach(EXIT_LOOP, Pthread::L4::cap(_pthreads[i])));
    _irq->trigger();
    L4Re::chksys(pthread_join(_pthreads[i], nullptr));
    _pthreads[i] = nullptr;
    _running[i] = false;
  }

protected:
  void start_thread(unsigned thread_num)
  {
    if (thread_num >= _pthreads.size())
      throw L4::Runtime_error(L4_ERANGE);

    void *param = (void *)&_disp[thread_num];

    pthread_t thr;
    L4Re::chksys(pthread_create(&thr, nullptr, nullptr, nullptr));
    L4Re::chksys(Pthread::L4::start(thr, run, param));

    _pthreads[thread_num] = thr;
    _running[thread_num] = true;
  }

private:
  std::array<pthread_t, NUM_THREADS> _pthreads;
  std::array<bool, NUM_THREADS> _running;
  L4Re::Util::Auto_del_cap<L4::Irq>::Cap _irq;


  static void *run(void *disp)
  {
    L4::Server<> server(l4_utcb());
    try
      {
        server.loop_noexc((Dispatcher *)disp);
      }
    catch (Cancel_exception &ce)
      {
      }

    return nullptr;
  }

  struct Dispatcher
  {
    l4_umword_t label_expectation = DEFAULT_LABEL;
    int recv_del_irq = 0;

    l4_msgtag_t dispatch(l4_msgtag_t, l4_umword_t label, l4_utcb_t *)
    {
      if ((label & ~0x3UL) == EXIT_LOOP)
        throw Cancel_exception();

      if (label == 0xdeadc)
        {
          recv_del_irq++;
          return l4_msgtag(-L4_ENOREPLY, 0, 0, 0);
        }

      if ((label_expectation >> 2) == (label >> 2))
        if (recv_del_irq--)
            return l4_msgtag(0x110, 0, 0, 0);
        else
          return l4_msgtag(PING_VALUE, 0, 0, 0);

      return l4_msgtag(-L4_EBADPROTO, 0, 0, 0);
    }
  };

  std::array<Dispatcher, NUM_THREADS>_disp;

  // Exception to exit the server loop
  class Cancel_exception {};
}; // class Recv_thread

} // namespace
