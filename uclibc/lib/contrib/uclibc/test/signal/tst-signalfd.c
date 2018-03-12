/* vi: set sw=4 ts=4 sts=4: */
/*
 * signalfd test for uClibc
 * Copyright (C) 2012 by Kevin Cernekee <cernekee@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/fcntl.h>

static int
do_test(void)
{
	int fd, ret, result = 0;
	struct signalfd_siginfo ssi;
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	fd = signalfd(-1, &mask, SFD_NONBLOCK);
	if (fd < 0) {
		printf("signalfd() failed: %s\n", strerror(errno));
		result = 1;
	}

	/* this should return immediately with EAGAIN due to SFD_NONBLOCK */
	memset(&ssi, 0, sizeof(ssi));
	ret = read(fd, &ssi, sizeof(ssi));
	if (ret != -1 || errno != EAGAIN) {
		error(0, 0, "first read() returned %d", ret);
		result = 1;
	}

	kill(getpid(), SIGUSR1);

	/* this should return a struct ssi indicating receipt of SIGUSR1 */
	ret = read(fd, &ssi, sizeof(ssi));
	if (ret != sizeof(ssi)) {
		error(0, 0, "second read() returned %d", ret);
		result = 1;
	}

	if (ssi.ssi_signo != SIGUSR1) {
		error(0, 0, "ssi contains bogus signo");
		result = 1;
	}

	return result;
}

#define TIMEOUT 5
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
