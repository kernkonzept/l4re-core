/*
 * NOOP POSIX signal backend
 * 
 * (c) 2013 Bjoern Doebel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

extern "C"
sighandler_t signal(int, sighandler_t) L4_NOTHROW
{
  errno = EINVAL;
  return SIG_ERR;
}

extern "C"
sighandler_t bsd_signal(int, sighandler_t) L4_NOTHROW
{
  errno = EINVAL;
  return SIG_ERR;
}

extern "C"
int sigaction(int, const struct sigaction *, struct sigaction *) L4_NOTHROW
{
  errno = EINVAL;
  return -1;
}

extern "C"
int sigprocmask(int, const sigset_t *, sigset_t *) noexcept
{
  errno = EINVAL;
  return -1;
}

extern "C"
int sigpending(sigset_t *) noexcept
{
  errno = EFAULT;
  return -1;
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
unsigned int alarm(unsigned int) noexcept
{
  return 0;
}

#include <l4/util/util.h>

int pause(void)
{
  l4_sleep_forever();
  errno = EINTR;
  return -1;
}

int setitimer(__itimer_which_t __which,
              const struct itimerval *__restrict __new,
              struct itimerval *__restrict __old) L4_NOTHROW
{
  (void)__which;
  (void)__new;
  (void)__old;
  errno = EINVAL;
  return -1;
}
