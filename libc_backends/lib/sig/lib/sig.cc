/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <l4/sys/thread>
#include <l4/sys/debugger.h>
#include <l4/sys/exception>
#include <l4/re/env>
#include <l4/re/debug>
#include <l4/util/util.h>
#include <l4/libc_backends/sig.h>

#include <sys/time.h>

#include "arch.h"

#include <errno.h>
#include <signal.h>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread-l4.h>

#include <l4/sys/cxx/ipc_epiface>
#include <l4/sys/cxx/ipc_server_loop>

namespace {
struct Sig_handling : L4::Epiface_t<Sig_handling, L4::Exception>
{
  // handlers registered with 'signal'
  struct sigaction sigactions[_NSIG];

  L4::Cap<L4::Thread> thcap;
  pthread_t pthread;

  struct itimerval current_itimerval;
  l4_cpu_time_t alarm_timeout;

  Sig_handling();

  void ping_exc_handler();
  l4_addr_t get_handler(int signum);
  int get_any_async_handler();
  bool is_async_sig(int sig);
  sighandler_t signal(int signum, sighandler_t handler) throw();
  int sigaction(int signum, const struct sigaction *act,
                struct sigaction *oldact) throw();
  int setitimer(__itimer_which_t __which,
                __const struct itimerval *__restrict __new,
                struct itimerval *__restrict __old) throw();

public:
  int op_exception(L4::Exception::Rights, l4_exc_regs_t &exc,
                   L4::Ipc::Opt<L4::Ipc::Snd_fpage> &);
};

}

// -----------------------------------------------------------------------


l4_addr_t
Sig_handling::get_handler(int signum)
{
  if (signum >= _NSIG)
    return 0;
  if (   (sigactions[signum].sa_flags & SA_SIGINFO)
      && sigactions[signum].sa_sigaction)
    return (l4_addr_t)sigactions[signum].sa_sigaction;
  else if (sigactions[signum].sa_handler)
    return (l4_addr_t)sigactions[signum].sa_handler;
  return 0;
}

bool
Sig_handling::is_async_sig(int sig)
{
  switch (sig)
    {
    case SIGALRM:
    case SIGHUP:
    case SIGUSR1:
    case SIGUSR2:
    case SIGWINCH:
    case SIGPWR:
    case SIGXCPU:
    case SIGSYS: return true;
    default: return false;
    };
}

int
Sig_handling::get_any_async_handler()
{
  for (int i = 0; i < _NSIG; ++i)
    if (is_async_sig(i) && get_handler(i))
      return i;
  return 0;
}

asm(
".text                           \n\t"
".global libc_be_sig_return_trap \n\t"
"libc_be_sig_return_trap:        \n\t"
#if defined(ARCH_x86) || defined(ARCH_amd64)
"                          ud2a  \n\t"
#elif defined(ARCH_arm)
".p2align 2                      \n\t"
"word: .long                    0xe1600070 \n\t" // smc
#elif defined(ARCH_arm64)
".p2align 2                      \n\t"
"brk #123                        \n\t"
#elif defined(ARCH_ppc32)
"trap                            \n\t"
#elif defined(ARCH_sparc)
"ta 0x1                          \n\t"
#elif defined(ARCH_mips)
".long 0x04170010 # sigrie 0x10  \n\t"
#else
#error Unsupported arch!
#endif
".previous                       \n\t"
);

extern char libc_be_sig_return_trap[];

static bool range_ok(l4_addr_t start, unsigned long size)
{
  l4_addr_t offset;
  unsigned flags;
  L4::Cap<L4Re::Dataspace> ds;

  return !L4Re::Env::env()->rm()->find(&start, &size, &offset, &flags, &ds)
         && !(flags & L4Re::Rm::Read_only);
}

static void dump_rm()
{
  L4::Cap<L4Re::Debug_obj> d(L4Re::Env::env()->rm().cap());
  d->debug(0);
}

static bool setup_sig_frame(l4_exc_regs_t *u, int signum)
{
#if defined(ARCH_mips)
  // put state + pointer to it on stack
  ucontext_t *ucf = (ucontext_t *)(u->r[29] - sizeof(*ucf));
#else
  // put state + pointer to it on stack
  ucontext_t *ucf = (ucontext_t *)(u->sp - sizeof(*ucf));
#endif

  /* Check if memory access is fine */
  if (!range_ok((l4_addr_t)ucf, sizeof(*ucf)))
    return false;

  fill_ucontext_frame(ucf, u);

#ifdef ARCH_arm
  u->sp = (l4_umword_t)ucf;
  u->r[0] = signum;
  u->r[1] = 0; // siginfo_t pointer, we do not have one right currently
  u->r[2] = (l4_umword_t)ucf;
  u->ulr  = (unsigned long)libc_be_sig_return_trap;
#elif defined(ARCH_mips)
  u->r[29] = (l4_umword_t)ucf;
  u->r[0] = signum;
  u->r[1] = 0; // siginfo_t pointer, we do not have one right currently
  u->r[2] = (l4_umword_t)ucf;
  u->epc  = (unsigned long)libc_be_sig_return_trap;
#else
  u->sp = (l4_umword_t)ucf - sizeof(void *);
  *(l4_umword_t *)u->sp = (l4_umword_t)ucf;

  // siginfo_t pointer, we do not have one right currently
  u->sp -= sizeof(siginfo_t *);
  *(l4_umword_t *)u->sp = 0;

  // both types get the signum as the first argument
  u->sp -= sizeof(l4_umword_t);
  *(l4_umword_t *)u->sp = signum;

  u->sp -= sizeof(l4_umword_t);
  *(unsigned long *)u->sp = (unsigned long)libc_be_sig_return_trap;
#endif

  return true;
}

int Sig_handling::op_exception(L4::Exception::Rights, l4_exc_regs_t &exc,
                               L4::Ipc::Opt<L4::Ipc::Snd_fpage> &)
{
  l4_addr_t handler;
  l4_exc_regs_t _u = exc;
  l4_exc_regs_t *const u = &_u;
  int pc_delta = 0;

#if defined(ARCH_arm) || defined(ARCH_mips)
  pc_delta = -4;
#endif

#if defined(ARCH_arm) || defined(ARCH_arm64)
  if ((u->err >> 26) == 0x3e)
#elif defined(ARCH_mips)
  if ((u->cause & 0x1ff) == 0x101)
#elif defined(ARCH_ppc32)
  if ((u->err & 3) == 4)
#else
  if (u->trapno == 0xff)
#endif
    {
      //printf("SIGALRM\n");

      int sig = get_any_async_handler();

      if (sig == 0)
        {
          printf("No signal handler found\n");
          return -L4_ENOREPLY;
        }

      if (   !(handler = get_handler(sig))
          || !setup_sig_frame(u, sig))
        {
          printf("Invalid user memory for sigframe...\n");
          return -L4_ENOREPLY;
        }


      l4_utcb_exc_pc_set(u, handler);
      exc = _u; // expensive? how to set amount of words in tag without copy?
      return -L4_EOK;
    }

  // x86: trap6
  if (l4_utcb_exc_pc(u) + pc_delta == (l4_addr_t)libc_be_sig_return_trap)
    {
      // sig-return
      //printf("Sigreturn\n");

#if defined(ARCH_arm)
      ucontext_t *ucf = (ucontext_t *)u->sp;
#elif defined(ARCH_mips)
      ucontext_t *ucf = (ucontext_t *)u->r[29];
#else
      ucontext_t *ucf = (ucontext_t *)(u->sp + sizeof(l4_umword_t) * 3);
#endif

      if (!range_ok((l4_addr_t)ucf, sizeof(*ucf)))
        {
          dump_rm();
          printf("Invalid memory...\n");
          return -L4_ENOREPLY;
        }

      fill_utcb_exc(u, ucf);

      //show_regs(u);

      exc = _u; // expensive? how to set amount of words in tag without copy?
      return -L4_EOK;
    }

  if (!(handler = get_handler(SIGSEGV)))
    {
      printf("No signal handler found\n");
      return -L4_ENOREPLY;
    }


  printf("Doing SIGSEGV\n");

  if (!setup_sig_frame(u, SIGSEGV))
    {
      printf("Invalid user memory for sigframe...\n");
      return -L4_ENOREPLY;
    }

  show_regs(u);

  l4_utcb_exc_pc_set(u, handler);
  exc = _u; // expensive? how to set amount of words in tag without copy?

  //printf("and back\n");
  return -L4_EOK;
}

static Sig_handling _sig_handling;

namespace {

struct Loop_hooks :
  public L4::Ipc_svr::Compound_reply,
  public L4::Ipc_svr::Default_setup_wait
{
  static l4_timeout_t timeout()
  {
    if (_sig_handling.alarm_timeout)
      {
	l4_timeout_t t;
	l4_rcv_timeout(l4_timeout_abs(_sig_handling.alarm_timeout, 1), &t);
	_sig_handling.alarm_timeout = 0;
	return t;
      }

    if (_sig_handling.current_itimerval.it_value.tv_sec == 0
	&& _sig_handling.current_itimerval.it_value.tv_usec == 0)
      return L4_IPC_NEVER;
    return l4_timeout(L4_IPC_TIMEOUT_NEVER,
	l4util_micros2l4to(_sig_handling.current_itimerval.it_value.tv_sec * 1000000 +
	  _sig_handling.current_itimerval.it_value.tv_usec));
  }

  void error(l4_msgtag_t res, l4_utcb_t *utcb)
  {
    long ipc_error = l4_ipc_error(res, utcb);

    if (ipc_error == L4_IPC_RETIMEOUT)
      {
	l4_msgtag_t t;

        // any thread is ok, right?!
	t = L4Re::Env::env()->main_thread()
            ->ex_regs(~0UL, ~0UL,
	              L4_THREAD_EX_REGS_TRIGGER_EXCEPTION);
	if (l4_error(t))
	  printf("ex_regs error\n");



	// reload
	_sig_handling.current_itimerval.it_value = _sig_handling.current_itimerval.it_interval;

	return;
      }
    printf("(unsupported/strange) loopabort: %lx\n", ipc_error);
  }
};


static void *__handler_main(void *)
{
  L4::Server<Loop_hooks> srv(l4_utcb());
  srv.loop_noexc(&_sig_handling);
  return 0;
}
}

Sig_handling::Sig_handling()
{
  if (pthread_create(&pthread, 0, __handler_main, 0))
    {
      fprintf(stderr, "libsig: Failed to create handler thread\n");
      return;
    }

  thcap = Pthread::L4::cap(pthread);

  l4_debugger_set_object_name(thcap.cap(), "&-");

  libsig_be_add_thread(l4re_env()->main_thread);

  return;
}

void libsig_be_set_dbg_name(const char *n)
{
  char s[15];
  snprintf(s, sizeof(s) - 1, "&%s", n);
  s[sizeof(s) - 1] = 0;
  l4_debugger_set_object_name(_sig_handling.thcap.cap(), s);
}

void libsig_be_add_thread(l4_cap_idx_t t)
{
  L4::Cap<L4::Thread> tt(t);
  L4::Thread::Attr a;
  a.exc_handler(_sig_handling.thcap);
  if (int e = l4_error(tt->control(a)))
    fprintf(stderr, "libsig: thread-control error: %d\n", e);
  //printf("Set exc-handler %lx for %lx\n", thcap.cap(), t);
}

inline
void Sig_handling::ping_exc_handler() throw()
{
  l4_ipc_call(thcap.cap(), l4_utcb(), l4_msgtag(0, 0, 0, 0), L4_IPC_NEVER);
}

inline
sighandler_t
Sig_handling::signal(int signum, sighandler_t handler) throw()
{
  if (signum < _NSIG)
    {
      sighandler_t old = sigactions[signum].sa_handler;
      sigactions[signum].sa_handler = handler;
      return old;
    }

  return SIG_ERR;
}

extern "C"
sighandler_t signal(int signum, sighandler_t handler) L4_NOTHROW
{
  //printf("Called: %s(%d, %p)\n", __func__, signum, handler);
  return _sig_handling.signal(signum, handler);
}

inline
int
Sig_handling::sigaction(int signum, const struct sigaction *act,
                        struct sigaction *oldact) throw()
{
  if (signum == SIGKILL || signum == SIGSTOP)
    return -EINVAL;

  if (signum < _NSIG)
    {
      if (oldact)
        *oldact = sigactions[signum];
      if (act)
        sigactions[signum] = *act;
      return 0;
    }

  return -EINVAL;
}

extern "C"
int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact) L4_NOTHROW
{
  //printf("Called: %s(%d, %p, %p)\n", __func__, signum, act, oldact);
  int err = _sig_handling.sigaction(signum, act, oldact);
  if (err < 0)
    {
      errno = -err;
      return -1;
    }

  return err;
}

extern "C"
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) throw()
{
  printf("%s(%d, %p, %p): Unimplemented\n", __func__, how, set, oldset);
  errno = EINVAL;
  return -1;
}

extern "C"
int sigpending(sigset_t *set) throw()
{
  printf("%s(%p): Unimplemented\n", __func__, set);
  errno = EFAULT;
  return -1;
}

int sigsuspend(const sigset_t *mask) throw()
{
  printf("%s(%p): Unimplemented\n", __func__, mask);
  errno = EFAULT;
  return -1;
}

extern "C"
int killpg(int pgrp, int sig) throw()
{
  printf("%s(%d, %d): Unimplemented\n", __func__, pgrp, sig);
  errno = EPERM;
  return -1;
}

extern "C"
unsigned int alarm(unsigned int seconds) L4_NOTHROW
{
  //printf("unimplemented: alarm(%u)\n", seconds);

  _sig_handling.alarm_timeout = l4_kip_clock(l4re_kip()) + seconds * 1000000;

  _sig_handling.ping_exc_handler();
  return 0;
}

extern "C"
pid_t wait(void *status)
{
  printf("unimplemented: wait(%p)\n", status);
  return -1;
}



int getitimer(__itimer_which_t __which,
              struct itimerval *__value) L4_NOTHROW
{
  if (__which != ITIMER_REAL)
    {
      errno = EINVAL;
      return -1;
    }

  *__value = _sig_handling.current_itimerval;

  _sig_handling.ping_exc_handler();
  return 0;
}

inline
int
Sig_handling::setitimer(__itimer_which_t __which,
                        __const struct itimerval *__restrict __new,
                        struct itimerval *__restrict __old) throw()
{
  printf("called %s(..)\n", __func__);

  if (__which != ITIMER_REAL)
    {
      errno = EINVAL;
      return -1;
    }

  if (__old)
    *__old = current_itimerval;

  if (__new->it_value.tv_usec < 0
      || __new->it_value.tv_usec > 999999
      || __new->it_interval.tv_usec < 0
      || __new->it_interval.tv_usec > 999999)
    {
      errno = EINVAL;
      return -1;
    }

  printf("%s: setting stuff\n", __func__);
  current_itimerval = *__new;

  ping_exc_handler();
  return 0;
}

int setitimer(__itimer_which_t __which,
              __const struct itimerval *__restrict __new,
              struct itimerval *__restrict __old) L4_NOTHROW
{
  int err = _sig_handling.setitimer(__which, __new, __old);
  if (err < 0)
    {
      errno = -err;
      return -1;
    }
  return 0;
}
