/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/utcb.h>
#include <l4/sys/types.h>
#include <l4/sys/factory>
#include <l4/sys/thread>
#include <l4/sys/cxx/ipc_server_loop>
#include <l4/re/env>
#include <l4/cxx/ipc_timeout_queue>

class Itas_br_manager : public L4::Ipc_svr::Server_iface
{
  enum : unsigned { Num_receive_buffers = 2 };

public:
  Itas_br_manager();

  int alloc_buffer_demand(L4::Type_info::Demand const &demand) override;
  L4::Cap<void> get_rcv_cap(int index) const override;
  int realloc_rcv_cap(int) override;
  int add_timeout(L4::Ipc_svr::Timeout *, l4_kernel_clock_t) override;
  int remove_timeout(L4::Ipc_svr::Timeout *) override;

protected:
  unsigned first_free_br() const
  { return Num_receive_buffers + 1; }

  void setup_wait(l4_utcb_t *utcb, L4::Ipc_svr::Reply_mode);

private:
  L4::Cap<void> rcv_cap_slots[Num_receive_buffers];
};

class Loop_hooks :
  public L4::Ipc_svr::Timeout_queue_hooks<Loop_hooks, Itas_br_manager>,
  public L4::Ipc_svr::Ignore_errors
{
public:
  /**
   * This function is required by Timeout_queue_hooks to get current time.
   */
  l4_kernel_clock_t now()
  { return l4_kip_clock(l4re_kip()); }
};

using Server = L4::Server<Loop_hooks>;

extern Server server;

class Dispatcher : public L4::Basic_registry
{
  /**
   * Handler class for stale requests from servers that have been
   * deregistered.
   */
  struct Null_handler : L4::Epiface_t<Null_handler, L4::Kobject>
  {};

  Null_handler _null_handler;

public:
  L4::Cap<void> register_obj(L4::Epiface *o);
  void unregister_obj(L4::Epiface *o);

  // Required by L4::Exc_dispatch via L4::Server::loop<>()
  l4_msgtag_t dispatch(l4_msgtag_t tag, l4_umword_t obj, l4_utcb_t *utcb);

private:
  L4::Cap<L4::Thread> _thread = L4Re::Env::env()->main_thread();
  L4::Cap<L4::Factory> _factory = L4Re::Env::env()->factory();
};

extern Dispatcher dispatcher;
