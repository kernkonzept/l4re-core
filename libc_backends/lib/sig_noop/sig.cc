/*
 * NOOP POSIX signal backend
 * 
 * (c) 2013 Bjoern Doebel <doebel@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <errno.h>
#include <signal.h>

extern "C"
sighandler_t signal(int, sighandler_t) L4_NOTHROW
{
  return SIG_ERR;
}

extern "C"
int sigaction(int, const struct sigaction *, struct sigaction *) L4_NOTHROW
{
  errno = EINVAL;
  return -1;
}

extern "C"
int sigprocmask(int, const sigset_t *, sigset_t *) throw()
{
  errno = EINVAL;
  return -1;
}

extern "C"
int sigpending(sigset_t *) throw()
{
  errno = EFAULT;
  return -1;
}

int sigsuspend(const sigset_t *) throw()
{
  errno = EFAULT;
  return -1;
}

extern "C"
int killpg(int, int) throw()
{
  errno = EPERM;
  return -1;
}

