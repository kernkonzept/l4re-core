/* vi: set sw=4 ts=4 sts=4: */
/*
 * Nonblocking socket test for uClibc
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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/fcntl.h>

static int
do_test(void)
{
	int fd, ret, result = 0;
	struct sockaddr_un sa;
	char buf;

	fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (fd < 0) {
		perror("socket()");
		result = 1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sun_family = AF_UNIX;
	strcpy(sa.sun_path, "socktest");
	unlink("socktest");
	if (bind(fd, (const struct sockaddr *)&sa, sizeof(sa)) < 0) {
		perror("bind()");
		result = 1;
	}

	ret = read(fd, &buf, sizeof(buf));
	if (ret != -1 || errno != EAGAIN) {
		error(0, 0, "Nonblocking read returned %d", ret);
		result = 1;
	}

	return result;
}

#define TIMEOUT 5
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
