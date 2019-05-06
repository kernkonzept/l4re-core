/*
 * Copyright (C) 2019 Kernkonzept GmbH.
 * Author(s): Yann Le Du <yann.le.du@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

/**
 * Tests that the libc backend no-op implementation can be called and linked
 * from its associated library.
 */

#include <signal.h>
#include <errno.h>

#include <l4/atkins/tap/main>

/**
 * 'signal()' can be called.
 */
TEST(SignalNoop, Signal)
{
  int sig = 1;
  sighandler_t handler = 0;

  errno = 0;
  EXPECT_EQ(SIG_ERR, signal(sig, handler)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

/**
 * 'sigaction()' can be called.
 */
TEST(SignalNoop, Sigaction)
{
  int sig = 1;
  struct sigaction sig_act;

  errno = 0;
  EXPECT_EQ(-1, sigaction(sig, &sig_act, &sig_act)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

/**
 * 'sigprocmask()' can be called.
 */
TEST(SignalNoop, Sigprocmask)
{
  int sig = 1;
  sigset_t sig_set;

  errno = 0;
  EXPECT_EQ(-1, sigprocmask(sig, &sig_set, &sig_set))
    << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

/**
 * 'sigpending()' can be called.
 */
TEST(SignalNoop, Sigpending)
{
  sigset_t sig_set;

  errno = 0;
  EXPECT_EQ(-1, sigpending(&sig_set)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

/**
 * 'sigsuspend()' can be called.
 */
TEST(SignalNoop, Sigsuspend)
{
  sigset_t sig_set;

  errno = 0;
  EXPECT_EQ(-1, sigsuspend(&sig_set)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}

/**
 * 'killpg()' can be called.
 */
TEST(SignalNoop, Killpg)
{
  int pgrp = 2;
  int sig = 1;

  errno = 0;
  EXPECT_EQ(-1, killpg(pgrp, sig)) << "Function is callable.";
  EXPECT_LT(0, errno) << "An error is signalled via errno.";
}
