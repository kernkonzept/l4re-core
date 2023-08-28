/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <signal.h>

#include <l4/cxx/avl_map>
#include <l4/cxx/dlist>
#include <l4/cxx/ipc_timeout_queue>
#include <l4/cxx/unique_ptr>
#include <l4/re/itas>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/sys/exception>
#include <l4/sys/thread>
#include <l4/sys/types.h>

#include "sig_arch.h"

class Signal_manager;

struct Sig_set
{
  sigset_t sigset;

  Sig_set() { del_all(); }
  Sig_set(sigset_t const &s) : sigset(s) {}
  Sig_set(Sig_set const &other) : sigset(other.sigset) {}

  Sig_set &operator=(Sig_set const &other)
  {
    sigset = other.sigset;
    return *this;
  }

  void add_all()
  { sigfillset(&sigset); }

  void del_all()
  { sigemptyset(&sigset); }

  void add_signal(int signum)
  { sigaddset(&sigset, signum); }

  void del_signal(int signum)
  { sigdelset(&sigset, signum); }

  bool has_signal(int signum) const
  { return sigismember(&sigset, signum) == 1; }

  bool empty() const
  { return sigisemptyset(&sigset) != 0; }

  explicit operator bool() const
  { return !empty(); }

  Sig_set operator~() const
  {
    sigset_t ret;
    // No public API to do that... :(
    // Need to be flexible for different C libs ... currently supports MUSL and
    // uclibc-ng
#ifndef _SIGSET_NWORDS
# define __L4_SIGSET_MEMBER __bits
# define __L4_SIGSET_NWORDS (128 / (8 * sizeof(*sigset.__bits)))
#else
# define __L4_SIGSET_MEMBER __val
# define __L4_SIGSET_NWORDS _SIGSET_NWORDS
#endif
    for (unsigned i = 0; i < __L4_SIGSET_NWORDS; i++)
      ret.__L4_SIGSET_MEMBER[i] = ~sigset.__L4_SIGSET_MEMBER[i];
#undef __L4_SIGSET_MEMBER
#undef __L4_SIGSET_NWORDS
    return ret;
  }

  Sig_set operator|(Sig_set const &other) const
  {
    sigset_t ret;
    sigorset(&ret, &sigset, &other.sigset);
    return ret;
  }

  Sig_set operator&(Sig_set const &other) const
  {
    sigset_t ret;
    sigandset(&ret, &sigset, &other.sigset);
    return ret;
  }

  Sig_set &operator|=(Sig_set const &other)
  {
    sigorset(&sigset, &sigset, &other.sigset);
    return *this;
  }

  Sig_set &operator&=(Sig_set const &other)
  {
    sigandset(&sigset, &sigset, &other.sigset);
    return *this;
  }

  operator sigset_t() const { return sigset; }
};

struct Pending_signal : cxx::D_list_item
{
  Pending_signal() = default;
  Pending_signal(siginfo_t const &si, Sig_arch_context const &arch,
                 l4_umword_t pfa)
  : si(si), arch(arch), pfa(pfa)
  {}

  /**
   * Is this a synchronous signal?
   *
   * Synchronous signals are created by the execution of the thread and are
   * caused by some specific instruction that was executed.
   */
  bool is_synchronous() const
  {
    return si.si_signo == SIGSEGV || si.si_signo == SIGBUS
           || si.si_signo == SIGILL || si.si_signo == SIGTRAP
           || si.si_signo == SIGFPE;
  }

  siginfo_t si;
  Sig_arch_context arch;
  l4_umword_t pfa = 0;
};

/**
 * List of pending signals for a particular thread or the process.
 *
 * Standard signals (below __SIGRTMIN) can only be queued once. Additional
 * instances are silently dropped. Realtime signals are queued in FIFO order
 * but still ordered by signal number. POSIX specifies that lowest numbered RT
 * signals are delivered first.
 *
 * Synchronous signals (i.e. which are caused by the thread itself, e.g.
 * SIGSEGV) are prioritized and delivered first. Other than that, the delivery
 * is ordered by the signal number (lowest first). RT signals with the same
 * number are delivered in FIFO order.
 */
class Pending_signal_list
{
public:
  bool queue_signal(siginfo_t const &si, Sig_arch_context const &arch,
                    l4_umword_t pfa);
  cxx::unique_ptr<Pending_signal> fetch_pending_signal(Sig_set const &blocked,
                                                       bool sync_first);

  Sig_set pending_set() const
  { return _sig_pending_set; }

private:
  void recalc_pending(int removed_signo);

  Sig_set _sig_pending_set;
  cxx::D_list<Pending_signal> _sig_pending_list;
  unsigned _num_rt_pending = 0;
};

using Thread_signals = L4::Kobject_2t<void, L4::Pager, L4::Exception>;

/**
 * Handle signal generation and delivery for a single thread.
 *
 * Holds the threads signal state and processes exceptions caused by the thread
 * execution to convert them to the appropriate signals.
 */
class Thread_signal_handler :
  public L4::Epiface_t<Thread_signal_handler, Thread_signals>
{
public:
  Thread_signal_handler(Signal_manager *mgr,
                        L4::Cap<L4::Thread> thread_cap,
                        l4_utcb_t *thread_utcb,
                        Thread_signal_handler *parent);

  l4_ret_t op_page_fault(L4::Pager::Rights rights, l4_umword_t addr, l4_umword_t pc,
                         L4::Ipc::Opt<L4::Ipc::Snd_fpage> &fp);

  l4_ret_t op_io_page_fault(L4::Io_pager::Rights,
                            l4_fpage_t io_pfa, l4_umword_t pc,
                            L4::Ipc::Opt<L4::Ipc::Snd_fpage> &);

  l4_ret_t op_exception(L4::Exception::Rights, l4_exc_regs_t &regs,
                        L4::Ipc::Opt<L4::Ipc::Snd_fpage> &fp);

  int sigaltstack(const struct sigaltstack &ss, struct sigaltstack &oss);

  void sigblock(sigset_t const &set);
  void sigunblock(sigset_t const &set);
  void sigsetmask(sigset_t const &set);

  Sig_set const &sigprocmask() const
  { return _sig_block; }

  Sig_set sigpending() const;

  int raise(int sig);

  void interrupt_thread();

  void stop_thread();

private:
  void return_from_signal(l4_exc_regs_t &regs);
  l4_ret_t deliver_pending_signals(l4_exc_regs_t &regs);
  l4_ret_t ensure_stack_writable(l4_umword_t pc, l4_addr_t top, l4_addr_t bottom);
  cxx::unique_ptr<Pending_signal> fetch_pending_signal();
  l4_ret_t call_default_action(siginfo_t const &si, l4_exc_regs_t const *regs);
  l4_ret_t stack_overflow(l4_addr_t sp);
  void dump_stack(L4Re::Util::Err const &err, l4_addr_t sp);
  bool needs_signal_delivery() const;

  l4_addr_t alt_stack() const
  {
    l4_addr_t bottom = reinterpret_cast<l4_addr_t>(_altstack.ss_sp);
    return (bottom + _altstack.ss_size) & ~l4_addr_t{Sig_stack_align - 1};
  }

  bool on_alt_stack(l4_addr_t sp) const
  {
    if (_altstack.ss_flags == SS_DISABLE)
      return false;

    l4_addr_t bottom = reinterpret_cast<l4_addr_t>(_altstack.ss_sp);
    l4_addr_t top = alt_stack();
    return bottom <= sp && sp <= top;
  }

  Signal_manager *_mgr;
  L4::Cap<L4::Thread> _thread;
  l4_utcb_t *_utcb;
  Sig_set _sig_block;
  Pending_signal_list _pending;
  struct sigaltstack _altstack;
  bool _thread_stopped = false;
};

/**
 * Per-process interval timer.
 */
class Interval_timer : public L4::Ipc_svr::Timeout_queue::Timeout
{
public:
  Interval_timer(Signal_manager *mgr)
  : _mgr(mgr)
  {}

  bool active() const { return _value != 0; }

  /**
   * Get absolute time of next expiration.
   */
  l4_cpu_time_t get_value() const { return _value; }

  /**
   * Get relative time between recurring expirations.
   *
   * A single-shot timer has a zero interval.
   */
  l4_cpu_time_t get_interval() const { return _interval; }

  /**
   * Arm timer.
   *
   * A previously armed timer is replaced when arming again.
   *
   * \param value     The absolute time of the next expiration.
   * \param interval  Recurrence interval or zero for a single-shot timer.
   */
  void set_timer(l4_cpu_time_t value, l4_cpu_time_t interval);

  /**
   * Disarm timer.
   */
  void clear_timer();

  void expired() override;

private:
  Signal_manager *_mgr;
  l4_cpu_time_t _value = 0, _interval = 0;
};

/**
 * Handle per-process signal state.
 *
 * Implements the L4Re::Itas interface for signal related application calls.
 * Also holds the per-process pending signal list and SIGALRM timer.
 */
class Signal_manager :
  public L4::Epiface_t<Signal_manager, L4Re::Itas>
{
  using Threads = cxx::Avl_map<l4_cap_idx_t, Thread_signal_handler*>;

public:
  Sig_set process_pending_set() const
  { return _pending.pending_set(); }

  cxx::unique_ptr<Pending_signal> fetch_pending_signal(Sig_set const &blocked)
  { return _pending.fetch_pending_signal(blocked, false); }

  bool send_process_signal(int signum);

  Thread_signal_handler *register_thread(L4::Cap<L4::Thread> thread_cap,
                                         l4_utcb_t *thread_utcb,
                                         Thread_signal_handler *parent = nullptr);

  l4_ret_t op_register_thread(L4Re::Itas::Rights,
                              L4::Ipc::Snd_fpage parent,
                              L4::Ipc::Snd_fpage thread_cap,
                              l4_addr_t thread_utcb);

  l4_ret_t op_unregister_thread(L4Re::Itas::Rights,
                                L4::Ipc::Snd_fpage thread);

  l4_ret_t op_sigaction(L4Re::Itas::Rights,
                        int signum,
                        const struct sigaction &act,
                        struct sigaction &oldact);

  l4_ret_t op_sigaltstack(L4Re::Itas::Rights,
                          L4::Ipc::Snd_fpage thread,
                          const struct sigaltstack &ss,
                          struct sigaltstack &oss);

  l4_ret_t op_sigprocmask(L4Re::Itas::Rights,
                          L4::Ipc::Snd_fpage thread,
                          int how, sigset_t const &set, sigset_t &oldset);

  l4_ret_t op_sigpending(L4Re::Itas::Rights,
                         L4::Ipc::Snd_fpage thread,
                         sigset_t &set);

  l4_ret_t op_setitimer(L4Re::Itas::Rights,
                        int which,
                        const struct itimerval &new_value,
                        struct itimerval &old_value);

  l4_ret_t op_getitimer(L4Re::Itas::Rights,
                        int which,
                        struct itimerval &curr_value);

  l4_ret_t op_raise(L4Re::Itas::Rights,
                    L4::Ipc::Snd_fpage thread,
                    int sig);

  void stop_all_threads();

private:
  Thread_signal_handler *handler(L4::Ipc::Snd_fpage thread);

  Threads _threads;
  Pending_signal_list _pending;
  Interval_timer _itimer{this};
};
