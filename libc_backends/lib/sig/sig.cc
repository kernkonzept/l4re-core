/*
 * Copyright (C) 2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/**
 * \file
 * POSIX signal backend.
 */

#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#include <l4/re/env>
#include <l4/re/itas>
#include <l4/util/util.h>
#include <pthread-l4.h>
#include <pthread.h>


extern "C"
sighandler_t signal(int signum, sighandler_t handler) L4_NOTHROW
{
  auto itas = L4Re::Env::env()->itas();
  if (!itas)
    {
      errno = -ENOSYS;
      return SIG_ERR;
    }

  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);

  /*
   * The behaviour of signal() is largely implementation defined, except for
   * SIG_DFL and SIG_IGN. Historically, System V semantics correspont to
   * `SA_RESETHAND | SA_NODEFER`. BSD changed the semantic to a much more sane
   * `SA_RESTART`. Linux provides the BSD semantics for signal() by default,
   * so should we.
   */
  sa.sa_flags = SA_RESTART;

  int ret = itas->sigaction(signum, &sa, &sa);
  if (ret < 0)
    errno = -ret;

  return ret >= 0 ? sa.sa_handler : SIG_ERR;
}

extern "C"
sighandler_t bsd_signal(int signum, sighandler_t handler) L4_NOTHROW
{
  return signal(signum, handler);
}

extern "C"
int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact) L4_NOTHROW
{
  // Dummy IPC parameter in case the caller passed a NULL pointer.
  struct sigaction sentinel;
  sentinel.sa_flags = L4Re::Itas::Ignore_sigaction;

  auto itas = L4Re::Env::env()->itas();
  if (!itas)
    {
      errno = -ENOSYS;
      return -1;
    }

  int ret = itas->sigaction(signum, act ? act : &sentinel,
                            oldact ? oldact : &sentinel);

  if (ret < 0)
    errno = -ret;

  return ret >= 0 ? 0 : -1;
}

extern "C"
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) noexcept
{
  // Dummy IPC parameter in case the caller passed a NULL pointer.
  sigset_t sentinel;

  auto itas = L4Re::Env::env()->itas();
  if (!itas)
    {
      errno = -ENOSYS;
      return -1;
    }

  auto self = Pthread::L4::cap(pthread_self());
  int ret = itas->sigprocmask(self, set ? how : -1,
                              set ? set : &sentinel,
                              oldset ? oldset : &sentinel);

  if (ret < 0)
    errno = -ret;

  return ret >= 0 ? 0 : -1;
}

extern "C"
int sigpending(sigset_t *set) noexcept
{
  auto itas = L4Re::Env::env()->itas();
  if (!itas)
    {
      errno = -ENOSYS;
      return -1;
    }

  auto self = Pthread::L4::cap(pthread_self());
  int ret = itas->sigpending(self, set);
  if (ret < 0)
    errno = -ret;

  return ret >= 0 ? 0 : -1;
}

int sigsuspend(const sigset_t *)
{
  errno = EFAULT;
  return -1;
}

int sigwait([[maybe_unused]] const sigset_t *set,
            [[maybe_unused]] int *sig)
{
  errno = EINVAL;
  return -1;
}

extern "C"
int killpg(int, int) noexcept
{
  errno = EPERM;
  return -1;
}

extern "C"
unsigned int alarm(unsigned int seconds) noexcept
{
  struct itimerval val;

  val.it_value.tv_sec = seconds;
  val.it_value.tv_usec = 0;
  val.it_interval.tv_sec = 0;
  val.it_interval.tv_usec = 0;

  struct itimerval old;

  if (setitimer(ITIMER_REAL, &val, &old) < 0)
    return 0;

  return old.it_value.tv_sec + (old.it_value.tv_usec ? 1 : 0);
}

int pause(void)
{
  l4_sleep_forever();
  errno = EINTR;
  return -1;
}

int setitimer(__itimer_which_t which,
              const struct itimerval *__restrict new_val,
              struct itimerval *__restrict old_val) L4_NOTHROW
{
  auto itas = L4Re::Env::env()->itas();
  if (!itas)
    {
      errno = -ENOSYS;
      return -1;
    }

  // Dummy IPC parameter in case the caller passed a NULL pointer.
  struct itimerval sentinel;

  int ret = itas->setitimer(which, new_val, old_val ? old_val : &sentinel);
  if (ret < 0)
    errno = -ret;

  return ret >= 0 ? 0 : -1;
}

int getitimer(__itimer_which_t which,
              struct itimerval *value) L4_NOTHROW
{
  auto itas = L4Re::Env::env()->itas();
  if (!itas)
    {
      errno = -ENOSYS;
      return -1;
    }

  int ret = itas->getitimer(which, value);
  if (ret < 0)
    errno = -ret;

  return ret >= 0 ? 0 : -1;
}

int sigaltstack(const struct sigaltstack *ss,
                struct sigaltstack *oss) L4_NOTHROW
{
  auto itas = L4Re::Env::env()->itas();
  if (!itas)
    {
      errno = -ENOSYS;
      return -1;
    }

  // Dummy IPC parameter in case the caller passed a NULL pointer.
  struct sigaltstack sentinel = {0, 0, 0};
  sentinel.ss_flags = -1;

  auto self = Pthread::L4::cap(pthread_self());
  int ret = itas->sigaltstack(self, ss ? ss : &sentinel,
                              oss ? oss : &sentinel);
  if (ret < 0)
    errno = -ret;

  return ret >= 0 ? 0 : -1;
}

extern "C" int l4re_raise(int sig);
int l4re_raise(int sig)
{
  auto itas = L4Re::Env::env()->itas();
  if (!itas)
    {
      errno = -ENOSYS;
      return -1;
    }

  auto self = Pthread::L4::cap(pthread_self());
  int ret = itas->raise(self, sig);
  if (ret < 0)
    errno = -ret;

  return ret >= 0 ? 0 : -1;
}
