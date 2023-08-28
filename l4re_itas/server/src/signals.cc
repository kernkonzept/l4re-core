/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/ucontext.h>
#include <ucontext.h>
#include <unistd.h>

#include <l4/re/dbg_events>
#include <l4/re/error_helper>

#include "dispatcher.h"
#include "globals.h"
#include "region.h"
#include "remote_access.h"
#include "signals.h"

// Signal 0 does not really exist and we don't store an action for it...
static struct sigaction sig_actions[_NSIG - 1];
static_assert(SIG_DFL == nullptr, "SIG_DFL is a nullptr");

bool
Pending_signal_list::queue_signal(siginfo_t const &si,
                                  Sig_arch_context const &arch,
                                  l4_umword_t pfa)
{
  auto sigrtmin = SIGRTMIN;

  // We follow the Linux implementation here. Only RT signals need to be queued
  // more than once. Standard signals only have two states: pending or
  // not-pending. The signal info of standard signals is not overwritten if
  // made pending a second time.
  if (si.si_signo < sigrtmin && _sig_pending_set.has_signal(si.si_signo))
    return true;

  // Limit the number of RT signals to prevent resource exhaustion.
  if (si.si_signo >= sigrtmin && _num_rt_pending >= _POSIX_SIGQUEUE_MAX)
    return false;

  auto it = _sig_pending_list.begin();
  while (it != _sig_pending_list.end() && it->si.si_signo <= si.si_signo)
    ++it;

  _sig_pending_list.insert_before(new Pending_signal(si, arch, pfa), it);
  _sig_pending_set.add_signal(si.si_signo);

  if (si.si_signo >= sigrtmin)
    _num_rt_pending++;

  return true;
}

cxx::unique_ptr<Pending_signal>
Pending_signal_list::fetch_pending_signal(Sig_set const &blocked,
                                          bool sync_first)
{
  if (_sig_pending_list.empty())
    return {};

  // First round: see if a synchronous signal is pending. They are caused by
  // the thread itself and need to be delivered at the causing instruction
  // immediately.
  if (sync_first)
    for (auto *ps : _sig_pending_list)
      {
        if (!ps->is_synchronous())
          continue;
        if (blocked.has_signal(ps->si.si_signo))
          continue;

        _sig_pending_list.remove(ps);
        recalc_pending(ps->si.si_signo);
        return cxx::unique_ptr<Pending_signal>(ps);
      }

  // Then consider all the others...
  for (auto *ps : _sig_pending_list)
    {
      if (blocked.has_signal(ps->si.si_signo))
        continue;

      _sig_pending_list.remove(ps);
      recalc_pending(ps->si.si_signo);
      return cxx::unique_ptr<Pending_signal>(ps);
    }

  return {};
}

void
Pending_signal_list::recalc_pending(int removed_signo)
{
  if (removed_signo >= SIGRTMIN)
    {
      _num_rt_pending--;

      for (auto *ps : _sig_pending_list)
        if (ps->si.si_signo == removed_signo)
          return;
    }

  _sig_pending_set.del_signal(removed_signo);
}

// --------------------------------------------------------------------------

Thread_signal_handler::Thread_signal_handler(Signal_manager *mgr,
                                             L4::Cap<L4::Thread> thread_cap,
                                             l4_utcb_t *thread_utcb,
                                             Thread_signal_handler *parent)
: _mgr(mgr), _thread(thread_cap), _utcb(thread_utcb)
{
  if (parent)
    // POSIX requires that newly created threads inherit the signal mask from
    // their parent.
    _sig_block = parent->_sig_block;

  _altstack.ss_sp = nullptr;
  _altstack.ss_flags = SS_DISABLE;
  _altstack.ss_size = 0;
}

l4_ret_t
Thread_signal_handler::op_page_fault(L4::Pager::Rights rights, l4_umword_t addr,
                                     l4_umword_t pc,
                                     L4::Ipc::Opt<L4::Ipc::Snd_fpage> &fp)
{
  // If the page fault could not be resolved, the fault will come back again as
  // exception IPC.
  return Global::local_rm->op_page_fault(rights, addr, pc, fp);
}

l4_ret_t
Thread_signal_handler::op_io_page_fault(L4::Io_pager::Rights rights,
                             l4_fpage_t io_pfa, l4_umword_t pc,
                             L4::Ipc::Opt<L4::Ipc::Snd_fpage> &fp)
{
  return Global::local_rm->op_io_page_fault(rights, io_pfa, pc, fp);
}

l4_ret_t
Thread_signal_handler::op_exception(L4::Exception::Rights, l4_exc_regs_t &regs,
                                    L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  // If the exception was caused by sigreturn(), it is the indication to
  // unwind the signal stack and resume normal execution. The precise error
  // code is irrelevant.
  if (l4_utcb_exc_pc(&regs) == reinterpret_cast<l4_addr_t>(&sigreturn))
    {
      return_from_signal(regs);
      if (!needs_signal_delivery())
        return L4_EOK;

      // There seem to be additional signals pending that we have to examine...
    }
  else
    {
      // Decipher what caused the exception IPC. Could be the thread execution
      // or ex_regs(L4_THREAD_EX_REGS_TRIGGER_EXCEPTION)...
      siginfo_t si;
      Sig_arch_context arch;
      memset(&si, 0, sizeof(si));
      switch (map_exception_to_signal(regs, &si, &arch))
        {
        case Exc_cause::Signal:
          {
            // Synchronous signals that are generated by the thread cannot be
            // ignored or blocked.
            l4_ret_t ret = L4_EOK;
            bool ignored = sig_actions[si.si_signo - 1].sa_handler == SIG_IGN;
            if (ignored || _sig_block.has_signal(si.si_signo))
              ret = call_default_action(si, &regs); // Won't return for sync signals
            else if (!_pending.queue_signal(si, arch, regs.pfa))
              ret = call_default_action(si, &regs); // Cannot happen but be defensive.

            if (ret < 0)
              return ret;

            break;
          }

        case Exc_cause::Unknown:
          {
            Err err;
            err.printf("%s: Unhandled exception: PC=0x%lx PFA=0x%lx LdrFlgs=0x%lx\n",
                       Global::l4re_aux->binary, l4_utcb_exc_pc(&regs),
                       l4_utcb_exc_pfa(&regs), Global::l4re_aux->ldr_flags);

            dump_exception_state(err, &regs);
            dump_stack(err, regs.sp);

            // FIXME: is this really the right approach?
            return -L4_ENOREPLY;
          }

        case Exc_cause::Ex_regs:
          // We interrupted the thread to dispatch pending asynchronous
          // signals.

          if (_thread_stopped)
            return -L4_ENOREPLY;

          break;
        }
    }

  return deliver_pending_signals(regs);
}

void
Thread_signal_handler::return_from_signal(l4_exc_regs_t &regs)
{
  // The stack layout is known at this stage. It is prepared by
  // deliver_pending_signals().
  ucontext_t *ucf = reinterpret_cast<ucontext_t *>(regs.sp);
  siginfo_t *si = reinterpret_cast<siginfo_t *>(ucf + 1);
  void *sig_utcb = static_cast<void *>(si + 1);

  // Restore original UTCB state before signal handler was entered.
  memcpy(_utcb, sig_utcb, L4_UTCB_OFFSET);

  // Restore original register state. The application is free to modify the
  // user context so we have to take it from the signal stack.
  fill_utcb_exc(&regs, ucf);

  // Restore original signal mask as mandated per POSIX. Note that the signal
  // handler is allowed to modify uc_sigmask, in which case this applies the
  // modified signal mask.
  _sig_block = ucf->uc_sigmask;
}

l4_ret_t
Thread_signal_handler::deliver_pending_signals(l4_exc_regs_t &regs)
{
  while (auto ps = fetch_pending_signal())
    {
      struct sigaction &sa = sig_actions[ps->si.si_signo - 1];

      if (sa.sa_handler == SIG_IGN)
        continue;

      if (sa.sa_handler == SIG_DFL)
        {
          l4_ret_t ret = call_default_action(ps->si, &regs);
          if (ret < 0)
            return ret;
          continue; // in case the default action was "ignore"
        }

      bool altstack = (sa.sa_flags & SA_ONSTACK)
                      && _altstack.ss_flags != SS_DISABLE;
      l4_addr_t sig_stack = (altstack && !on_alt_stack(regs.sp))
                            ? alt_stack() : regs.sp;

      constexpr size_t sig_frame_size = L4_UTCB_OFFSET
                                        + sizeof(siginfo_t)
                                        + sizeof(ucontext_t);
      // TODO: We need to make an L4Re version of all signal headers. Right
      // now, they reflect what Linux does which is not necessarily correct
      // as the following static assert shows...
      //static_assert(sig_frame_size + 1024 < MINSIGSTKSZ,
      //              "Signal frame shall fit into minimal stack size");

      l4_addr_t sig_context = sig_adjust_stack(sig_stack - sig_frame_size);
      l4_addr_t sig_bottom = l4_trunc_page(sig_context);
      if (sig_bottom > sig_stack)
        return stack_overflow(sig_bottom);

      // The ITAS is the pager for the application stacks. Accessing the stack
      // thus requires them to be present. The external region manager of ITAS
      // does not know the stack dataspace and cannot help us in case we
      // trigger a page fault.
      l4_ret_t ret = ensure_stack_writable(l4_utcb_exc_pc(&regs), sig_stack,
                                           sig_bottom);
      if (ret < 0)
        return ret;

      // Make sure the structures are properly aligned...
      static_assert(alignof(ucontext_t) <= Sig_stack_align,
                    "Context must be at most stack aligned");
      static_assert(alignof(ucontext_t) >= alignof(siginfo_t),
                    "Context must be at least as much aligned as signal info");

      // Establish signal stack frame. In return_from_signal(), the same layout
      // is expected too.
      ucontext_t *ucf = reinterpret_cast<ucontext_t *>(sig_context);
      siginfo_t *si = reinterpret_cast<siginfo_t *>(ucf + 1);
      void *sig_utcb = static_cast<void *>(si + 1);

      fill_ucontext_frame(ucf, &regs, ps->arch, ps->pfa);
      *si = ps->si;
      memcpy(sig_utcb, _utcb, L4_UTCB_OFFSET);

      setup_sighandler_frame(&regs, ucf, si, sa);

      // During delivery, the signal is blocked so that the signal handler does
      // not interrupt itself. The application can opt out of this when setting
      // the SA_NODEFER flag, though. In case of one-shot handlers, the signal
      // must not be blocked either.
      ucf->uc_sigmask = _sig_block.sigset;
      _sig_block |= sa.sa_mask;
      if ((sa.sa_flags & (SA_NODEFER | SA_RESETHAND)) == 0)
        _sig_block.add_signal(ps->si.si_signo);

      // SA_RESETHAND -> one-shot signal handler. Restore to default handler
      // on first delivery.
      if (sa.sa_flags & SA_RESETHAND)
        {
          sa.sa_handler = SIG_DFL;
          sigemptyset(&sa.sa_mask);
          sa.sa_flags = 0;
        }
    }

  return L4_EOK;
}

/**
 * Make sure the pages between `top` and `bottom` are writable.
 */
l4_ret_t
Thread_signal_handler::ensure_stack_writable(l4_umword_t pc, l4_addr_t top,
                                             l4_addr_t bottom)
{
  for (l4_umword_t p = l4_trunc_page(top); ; p -= L4_PAGESIZE)
    {
      L4::Ipc::Opt<L4::Ipc::Snd_fpage> rfp;
      // map writable
      l4_ret_t ret = Global::local_rm->op_page_fault(L4_CAP_FPAGE_W, p | 2, pc,
                                                     rfp);
      if (ret < 0)
        return stack_overflow(p);

      if (p <= bottom)
        break;
    }

  return L4_EOK;
}

int
Thread_signal_handler::sigaltstack(const struct sigaltstack &ss,
                                   struct sigaltstack &oss)
{
  bool on_stack = false;
  if (_altstack.ss_flags != SS_DISABLE)
    {
      l4_umword_t ip = ~0UL;
      l4_umword_t sp = ~0UL;
      int err = l4_error(_thread->ex_regs(&ip, &sp, 0));
      if (err < 0)
        return err;

      on_stack = on_alt_stack(sp);
    }

  oss = _altstack;
  if (on_stack)
    oss.ss_flags = SS_ONSTACK;

  // Don't change anything, just return the current altstack settings.
  if (ss.ss_flags == -1)
    return L4_EOK;

  // Reject unknown flags.
  if (ss.ss_flags != 0 && ss.ss_flags != SS_DISABLE)
    return -EINVAL;

  // A new stack can only be established if the thread is not using the current
  // alternate stack.
  if (_altstack.ss_flags != SS_DISABLE && on_stack)
    return -EPERM;

  // Verify the new stack is large enough.
  if (ss.ss_flags == 0 && ss.ss_size < MINSIGSTKSZ)
    return -EINVAL;

  if (ss.ss_flags == SS_DISABLE)
    _altstack.ss_flags = SS_DISABLE;
  else
    _altstack = ss;

  return L4_EOK;
}

void
Thread_signal_handler::sigblock(sigset_t const &set)
{
  _sig_block |= set;
}

void
Thread_signal_handler::sigunblock(sigset_t const &set)
{
  _sig_block &= ~Sig_set(set);
  if (needs_signal_delivery())
    interrupt_thread();
}

void
Thread_signal_handler::sigsetmask(sigset_t const &set)
{
  _sig_block = set;
  if (needs_signal_delivery())
    interrupt_thread();
}

cxx::unique_ptr<Pending_signal>
Thread_signal_handler::fetch_pending_signal()
{
  auto ret = _pending.fetch_pending_signal(_sig_block, true);
  if (!ret)
    ret = _mgr->fetch_pending_signal(_sig_block);

  return ret;
}

Sig_set
Thread_signal_handler::sigpending() const
{
  Sig_set ret = _pending.pending_set();
  ret |= _mgr->process_pending_set();
  return ret;
}

int
Thread_signal_handler::raise(int signum)
{
  if (signum <= 0 || signum >= _NSIG)
    return -EINVAL;

  siginfo_t si;
  memset(&si, 0, sizeof(si));
  si.si_signo = signum;

  if (!_pending.queue_signal(si, {}, 0))
    return -ENOMEM;

  interrupt_thread();
  return 0;
}

void
Thread_signal_handler::interrupt_thread()
{
  // TODO: actually, we need something like L4_THREAD_EX_REGS_TRIGGER_INTERRUPT
  // to gracefully cancel ongoing IPC. This includes threads waiting
  // (l4_sleep()). Right now, signals do not interrupt POSIX system calls as
  // they should.
  _thread->ex_regs(~0UL, ~0UL, L4_THREAD_EX_REGS_TRIGGER_EXCEPTION);
}

l4_ret_t
Thread_signal_handler::call_default_action(siginfo_t const &si,
                                           l4_exc_regs_t const *regs)
{
  switch (si.si_signo)
    {
    case SIGABRT:
    case SIGBUS:
    case SIGFPE:
    case SIGILL:
    case SIGQUIT:
    case SIGSEGV:
    case SIGSYS:
    case SIGTRAP:
    case SIGXCPU:
    case SIGXFSZ:
    // All signals above should abort with a core dump.
    {
      Err err(Err::Fatal);

      _mgr->stop_all_threads();

      err.printf("Fatal signal: %s (si_signo=%d, si_code=%d)\n",
                 strsignal(si.si_signo), si.si_signo, si.si_code);

      if (   si.si_signo == SIGILL  || si.si_signo == SIGFPE
          || si.si_signo == SIGSEGV || si.si_signo == SIGBUS)
        err.printf("si_addr=%p\n", si.si_addr);
      else if (si.si_signo == SIGCHLD)
        err.printf("si_pid=%ld, si_status=%d, si_uid=%ld\n",
                   static_cast<long>(si.si_pid), si.si_status,
                   static_cast<long>(si.si_uid));

      if (Global::l4re_aux)
        err.printf("Binary: '%s'\n", Global::l4re_aux->binary);

      if (regs)
        {
          dump_exception_state(err, regs);
          dump_stack(err, regs->sp);
        }

      L4::Cap<L4Re::Dbg_events> dbg_events = L4Re::Env::env()->dbg_events();

      if (!dbg_events.is_valid())
        err.printf("No backtrace service available!\n");
      else
        {
          int r = dbg_events->request_backtrace(*regs, ra_if.obj_cap(),
                                                Global::local_rm->obj_cap());
          if (r == 0)
            // Let the faulting thread in its blocked state, l4re-itas to
            // return to its server loop
            return -L4_ENOREPLY;

          err.printf("Backtrace at %lx could not be requested: %d\n",
                     dbg_events.cap(), r);
        }

      _exit(128 + si.si_signo);
      break;
    }

    case SIGALRM:
    case SIGHUP:
    case SIGINT:
    case SIGKILL:
    case SIGPIPE:
    case SIGTERM:
    case SIGUSR1:
    case SIGUSR2:
    case SIGVTALRM:
      _exit(128 + si.si_signo);
      break;

    case SIGCONT:
    case SIGSTOP:
    case SIGTSTP:
    case SIGTTIN:
    case SIGTTOU:
      // TODO: implement Stop/Continue actions. Ignored so far.
      break;

    default:
      // All other signals are ignored by default.
      break;
    }

  return L4_EOK;
}

l4_ret_t
Thread_signal_handler::stack_overflow(l4_addr_t sp)
{
  Err().printf("Signal stack overflow!\n");
  siginfo_t si;
  si.si_signo = SIGSEGV;
  si.si_code = SEGV_MAPERR;
  si.si_addr = reinterpret_cast<void *>(sp);
  call_default_action(si, nullptr); // Won't return
  return -L4_ENOREPLY;
}

void
Thread_signal_handler::dump_stack(L4Re::Util::Err const &err, l4_addr_t sp)
{
  err.printf("\n");
  err.printf("Stack dump:\n");
  if (sp & (sizeof(l4_umword_t) - 1U))
    {
      err.printf("WARNING: unaligned stack!\n");
      sp &= ~l4_addr_t{sizeof(l4_umword_t) - 1};
    }

  auto stack_region = Global::local_rm->find(L4Re::Util::Region(sp, sp));
  if (!stack_region)
    {
      err.printf("--- invalid stack region ---\n");
      return;
    }

  // Dump at most 1k.
  unsigned long stack_size = stack_region->first.end() + 1U - sp;
  if (stack_size > 1024)
    stack_size = 1024;

  // Make sure the stack is readable.
  l4_addr_t sp_page = l4_trunc_page(sp);
  for (unsigned long remain = l4_round_page(stack_size + (sp - sp_page));
       remain >= L4_PAGESIZE;
       remain -= L4_PAGESIZE, sp_page += L4_PAGESIZE)
    {
      L4::Ipc::Opt<L4::Ipc::Snd_fpage> rfp;
      l4_ret_t ret = Global::local_rm->op_page_fault(L4_CAP_FPAGE_W, sp_page, 0,
                                                     rfp);
      if (ret < 0)
        {
          err.printf("--- unreadable stack region ---\n");
          return;
        }
    }

  unsigned constexpr word_size = sizeof(l4_umword_t);
  unsigned constexpr words_per_line = word_size == 4 ? 4 : 2;
  unsigned field_width = word_size * 2;
  l4_addr_t p = sp;
  while (stack_size > 0)
    {
      err.printf("%*lx:", field_width, p);
      for (unsigned i = 0;
           i < words_per_line && stack_size;
           i++, p += word_size, stack_size -= word_size)
        err.cprintf(" %*lx", field_width, *reinterpret_cast<l4_umword_t *>(p));
      err.cprintf("\n");
    }
}

bool
Thread_signal_handler::needs_signal_delivery() const
{
  auto pending = (_pending.pending_set() | _mgr->process_pending_set())
                  & ~_sig_block;
  return !pending.empty();
}

void
Thread_signal_handler::stop_thread()
{
  _thread_stopped = true;
  _thread->ex_regs(~0UL, ~0UL,
                   L4_THREAD_EX_REGS_CANCEL | L4_THREAD_EX_REGS_TRIGGER_EXCEPTION);
}

/***************************************************************************/

void
Interval_timer::expired()
{
  // That should never fail. If it does, there's nothing we can do anyway.
  _mgr->send_process_signal(SIGALRM);

  if (_interval)
    {
      l4_cpu_time_t now = l4_kip_clock(l4re_kip());
      l4_cpu_time_t next = timeout() + _interval;
      while (now >= next)
        next += _interval;

      _value = next;
      server.add_timeout(this, next);
    }
  else
    _value = 0;
}

void
Interval_timer::set_timer(l4_cpu_time_t value, l4_cpu_time_t interval)
{
  clear_timer();

  _value = value;
  _interval = interval;
  server.add_timeout(this, _value);
}

void
Interval_timer::clear_timer()
{
  if (_value)
    {
      server.remove_timeout(this);
      _value = _interval = 0;
    }
}

/***************************************************************************/

void
Signal_manager::stop_all_threads()
{
  for (auto &it : _threads)
    it.second->stop_thread();
}

bool
Signal_manager::send_process_signal(int signum)
{
  siginfo_t si;
  memset(&si, 0, sizeof(si));
  si.si_signo = signum;

  if (!_pending.queue_signal(si, {}, 0))
    return false;

  // Try to find a thread that can receive the signal. Theoretically, the found
  // destination thread could just be about to block the signal. In this case,
  // it might not be delivered in due time to another thread. This is deemed
  // extremely unlikely, though.
  for (auto &it : _threads)
    if (!it.second->sigprocmask().has_signal(signum))
      {
        it.second->interrupt_thread();
        break;
      }

  // If no thread was found, it means that all had blocked the signal. In this
  // case the signal is delivered to the first thread that unblocks it.

  return true;
}

Thread_signal_handler *
Signal_manager::register_thread(L4::Cap<L4::Thread> thread_cap,
                                l4_utcb_t *thread_utcb,
                                Thread_signal_handler *parent)
{
  auto node = _threads.find_node(thread_cap.cap());
  if (node)
    return node->second;

  auto hdl = cxx::make_unique<Thread_signal_handler>(this, thread_cap,
                                                     thread_utcb, parent);
  if (!dispatcher.register_obj(hdl.get()))
    return nullptr;

  if (_threads.insert(thread_cap.cap(), hdl.get()).second != 0)
    {
      dispatcher.unregister_obj(hdl.get());
      return nullptr;
    }

  return hdl.release();
}

l4_ret_t
Signal_manager::op_register_thread(L4Re::Itas::Rights,
                                   L4::Ipc::Snd_fpage parent,
                                   L4::Ipc::Snd_fpage thread_cap,
                                   l4_addr_t thread_utcb)
{
  if (!thread_cap.local_id_received())
    return -L4_EINVAL;

  auto child_cap = L4::Cap<L4::Thread>(thread_cap.base());
  l4_utcb_t *child_utcb = reinterpret_cast<l4_utcb_t *>(thread_utcb);
  Thread_signal_handler *sig_handler = register_thread(child_cap, child_utcb,
                                                       handler(parent));
  if (!sig_handler)
    return -L4_ENOMEM;

  L4::Thread::Attr attr;
  attr.bind(child_utcb, L4Re::This_task);
  attr.pager(sig_handler->obj_cap());
  attr.exc_handler(sig_handler->obj_cap());
  L4Re::chksys(child_cap->control(attr), "l4re_itas: Setup child thread.");

  return L4_EOK;
}

l4_ret_t
Signal_manager::op_unregister_thread(L4Re::Itas::Rights,
                                     L4::Ipc::Snd_fpage thread)
{
  if (!thread.local_id_received())
    return -L4_EINVAL;

  auto caller = L4::Cap<L4::Thread>(thread.base());
  auto node = _threads.find_node(caller.cap());
  if (!node)
    return -L4_EINVAL;

  dispatcher.unregister_obj(node->second);
  delete node->second;
  _threads.remove(node->first);

  return L4_EOK;
}

l4_ret_t
Signal_manager::op_sigaction(L4Re::Itas::Rights,
                             int signum,
                             const struct sigaction &act,
                             struct sigaction &oldact)
{
  if (signum <= 0 || signum >= _NSIG)
    return -EINVAL;

  // The SIGKILL and SIGSTOP signals cannot be caught or ignored.
  if (signum == SIGKILL || signum == SIGSTOP)
    return -EINVAL;

  auto old = sig_actions[signum - 1];
  old.sa_flags &= ~SA_RESETHAND;

  if (act.sa_flags != L4Re::Itas::Ignore_sigaction)
    {
      // Reject unknown or unhandled flags
      if (act.sa_flags & ~(SA_NOCLDSTOP | SA_NOCLDWAIT | SA_NODEFER
                           | SA_ONSTACK | SA_RESETHAND | SA_RESTART
                           | SA_SIGINFO))
        return -EINVAL;

      sig_actions[signum - 1] = act;
    }

  oldact = old;

  return L4_EOK;
}

l4_ret_t
Signal_manager::op_sigaltstack(L4Re::Itas::Rights,
                               L4::Ipc::Snd_fpage thread,
                               const struct sigaltstack &ss,
                               struct sigaltstack &oss)
{
  // The references point to the UTCB!
  struct sigaltstack new_stack = ss;
  struct sigaltstack old_stack;
  int ret = handler(thread)->sigaltstack(new_stack, old_stack);
  oss = old_stack;
  return ret;
}

l4_ret_t
Signal_manager::op_sigprocmask(L4Re::Itas::Rights,
                               L4::Ipc::Snd_fpage thread,
                               int how, sigset_t const &set, sigset_t &oldset)
{
  auto *hdl = handler(thread);

  Sig_set old = hdl->sigprocmask();

  // The SIGKILL and SIGSTOP signals cannot be blocked.
  Sig_set invalid_signals;
  invalid_signals.add_signal(SIGKILL);
  invalid_signals.add_signal(SIGSTOP);

  switch (how)
    {
    case SIG_BLOCK:
      hdl->sigblock(Sig_set(set) & ~invalid_signals);
      break;
    case SIG_UNBLOCK:
      hdl->sigunblock(set);
      break;
    case SIG_SETMASK:
      hdl->sigsetmask(Sig_set(set) & ~invalid_signals);
      break;
    case -1:
      // Nothing set, just query old sigprocmask.
      break;
    default:
      return -EINVAL;
    }

  oldset = old.sigset;
  return L4_EOK;
}

l4_ret_t
Signal_manager::op_sigpending(L4Re::Itas::Rights,
                              L4::Ipc::Snd_fpage thread,
                              sigset_t &set)
{
  auto *hdl = handler(thread);
  set = hdl->sigpending().sigset;
  return L4_EOK;
}

l4_ret_t
Signal_manager::op_setitimer(L4Re::Itas::Rights,
                             int which,
                             const struct itimerval &new_value,
                             struct itimerval &old_value)
{
  if (which != ITIMER_REAL)
    return -EINVAL;

  if (   new_value.it_value.tv_sec < 0
      || new_value.it_value.tv_usec < 0
      || new_value.it_value.tv_usec > 999999)
    return -EINVAL;

  if (   new_value.it_interval.tv_sec < 0
      || new_value.it_interval.tv_usec < 0
      || new_value.it_interval.tv_usec > 999999)
    return -EINVAL;

  l4_cpu_time_t now = l4_kip_clock(l4re_kip());

  if (_itimer.active())
    {
      l4_cpu_time_t next = _itimer.get_value();
      if (next < now)
        next = 0;
      else
        next -= now;

      old_value.it_value.tv_sec = next / 1000000;
      old_value.it_value.tv_usec = next % 1000000;
      old_value.it_interval.tv_sec = _itimer.get_interval() / 1000000;
      old_value.it_interval.tv_usec = _itimer.get_interval() % 1000000;
    }
  else
    {
      old_value.it_value.tv_sec = 0;
      old_value.it_value.tv_usec = 0;
      old_value.it_interval.tv_sec = 0;
      old_value.it_interval.tv_usec = 0;
    }

  if (new_value.it_value.tv_sec != 0 || new_value.it_value.tv_usec != 0)
    _itimer.set_timer(
      now + static_cast<l4_cpu_time_t>(new_value.it_value.tv_sec) * 1000000
          + new_value.it_value.tv_usec,
      static_cast<l4_cpu_time_t>(new_value.it_interval.tv_sec) * 1000000
        + new_value.it_interval.tv_usec);
  else
    _itimer.clear_timer();

  return L4_EOK;
}

l4_ret_t
Signal_manager::op_getitimer(L4Re::Itas::Rights,
                             int which,
                             struct itimerval &curr_value)
{
  if (which != ITIMER_REAL)
    return -EINVAL;

  l4_cpu_time_t now = l4_kip_clock(l4re_kip());

  if (_itimer.active())
    {
      l4_cpu_time_t next = _itimer.get_value();
      if (next < now)
        next = 0;
      else
        next -= now;

      curr_value.it_value.tv_sec = next / 1000000;
      curr_value.it_value.tv_usec = next % 1000000;
      curr_value.it_interval.tv_sec = _itimer.get_interval() / 1000000;
      curr_value.it_interval.tv_usec = _itimer.get_interval() % 1000000;
    }
  else
    {
      curr_value.it_value.tv_sec = 0;
      curr_value.it_value.tv_usec = 0;
      curr_value.it_interval.tv_sec = 0;
      curr_value.it_interval.tv_usec = 0;
    }

  return 0;
}

l4_ret_t
Signal_manager::op_raise(L4Re::Itas::Rights, L4::Ipc::Snd_fpage thread, int sig)
{
  return handler(thread)->raise(sig);
}

Thread_signal_handler *
Signal_manager::handler(L4::Ipc::Snd_fpage thread)
{
  if (!thread.local_id_received())
    L4Re::throw_error(-L4_EINVAL, "non-local cap received");

  auto caller = L4::Cap<L4::Thread>(thread.base());
  auto node = _threads.find_node(caller.cap());
  if (!node)
    L4Re::throw_error(-ESRCH, "unknown thread");

  return node->second;
}
