/* vi: set sw=4 ts=4 sts=4: */
/*
 * timerfd test for uClibc
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
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <sys/timerfd.h>
#include <sys/fcntl.h>

static int
do_test(void)
{
	int fd, ret, result = 0;
	struct itimerspec s;
	uint64_t val;
	time_t start, now;

	fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if (fd < 0) {
		perror("timerfd() failed");
		result = 1;
	}
	s.it_value.tv_sec = 1;
	s.it_value.tv_nsec = 0;
	s.it_interval.tv_sec = 0;
	s.it_interval.tv_nsec = 0;
	timerfd_settime(fd, 0, &s, NULL);
	start = time(NULL);

	/* this should return immediately with EAGAIN due to TFD_NONBLOCK */
	ret = read(fd, &val, sizeof(val));
	if (ret != -1 || errno != EAGAIN) {
		error(0, 0, "first read() returned %d", ret);
		result = 1;
	}

	/* let the timer expire, then check it again */
	do {
		now = time(NULL);
	} while (now - start < 2);

	ret = read(fd, &val, sizeof(val));
	if (ret != sizeof(val)) {
		error(0, 0, "second read() returned %d", ret);
		result = 1;
	}

	/* we are expecting a single expiration, since it_interval is 0 */
	if (val != 1) {
		error(0, 0, "wrong number of expirations: %" PRIx64, val);
		result = 1;
	}

	return result;
}

#define TIMEOUT 5
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
