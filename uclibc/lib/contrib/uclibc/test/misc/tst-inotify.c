/* vi: set sw=4 ts=4 sts=4: */
/*
 * inotify test for uClibc
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
#include <inttypes.h>
#include <sys/inotify.h>
#include <sys/fcntl.h>

static int
do_test(void)
{
	int ifd, fd, ret, result = 0;
	struct inotify_event e;
	char tfile[] = "/tmp/inotify.XXXXXX";

	fd = mkstemp(tfile);
	close(fd);

	ifd = inotify_init1(IN_NONBLOCK);
	if (ifd < 0) {
		perror("inotify_init1()");
		result = 1;
	}
	if (inotify_add_watch(ifd, tfile, IN_DELETE_SELF) < 0) {
		perror("inotify_add_watch()");
		result = 1;
	}

	/* nonblocking inotify should return immediately with no events */
	ret = read(ifd, &e, sizeof(e));
	if (ret != -1 || errno != EAGAIN) {
		error(0, 0, "first read() returned %d", ret);
		result = 1;
	}

	/* generate an event */
	unlink(tfile);

	/* now check whether our event was seen */
	ret = read(ifd, &e, sizeof(e));
	if (ret != sizeof(e)) {
		error(0, 0, "second read() returned %d", ret);
		result = 1;
	}

	if (!(e.mask & IN_DELETE_SELF)) {
		error(0, 0, "incorrect event mask: %" PRIx32, e.mask);
		result = 1;
	}

	return result;
}

#define TIMEOUT 5
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
