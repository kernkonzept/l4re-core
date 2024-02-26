/* SPDX-License-Identifier: GPL-2.0-only OR License-Ref-kk-custom */
/*
 * Copyright (C) 2023 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 */
#pragma once

#include <pthread.h>

#include <l4/re/util/br_manager>
#include <l4/re/util/object_registry>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/sys/irq>
#include <l4/sys/semaphore>
#include <l4/cxx/unique_ptr>

namespace Ned {

class Foreign_registry;

class Foreign_server_object : public L4::Epiface
{};

/**
 * Server loop with dedicated IPC registry that runs in a separate thread.
 *
 * It's main purpose is to have a separate thread that receives the exit
 * signals of child tasks. This is required to prevent potential deadlocks when
 * Ned calls IPC gates that are bound to child tasks.
 *
 * The implementation assumes that all IPC objects are registered from the main
 * thread. The IPC of these objects is then dispatched asynchronously in the
 * context of the Foreign_server thread. Unregistering IPC objects must be
 * done from the main thread too and will ensure that no dispatching for this
 * IPC object can happen after unregister_obj() returned. Calling the
 * associated registry in the context of the Foreign_server thread *will*
 * deadlock!
 */
class Foreign_server : public L4::Server<L4Re::Util::Br_manager_hooks>,
                       private L4::Irqep_t<Foreign_server>
{
  using Base = L4::Server<L4Re::Util::Br_manager_hooks>;
  friend struct L4::Irqep_t<Foreign_server>;
  friend class Foreign_registry;

public:
  Foreign_server();

  L4Re::Util::Object_registry *registry() const;

private:
  cxx::unique_ptr<Foreign_registry> _r;
  pthread_t _th;
  pthread_mutex_t _start_mutex; ///< Synchronize start of thread

  /**
   * L4::Irq to interrupt dispatching of IPC in thread.
   *
   * Used to make sure that no IPC dispatching is under way when registering or
   * unregistering IPC gates.
   */
  L4Re::Util::Unique_cap<L4::Irq> _interrupt;

  /**
   * Signal of dispatching thread to main thread that the _interrupt was
   * received and IPC dispatching has halted.
   */
  L4Re::Util::Unique_cap<L4::Semaphore> _ack;

  /**
   * Signal of main thread to dispatching thread that the IPC loop may resume.
   */
  L4Re::Util::Unique_cap<L4::Semaphore> _resume;

  static void *__run(void *);
  void run();

  void pause_dispatch();
  void resume_dispatch();
  void handle_irq();
};

extern Foreign_server *foreign_server;

}
