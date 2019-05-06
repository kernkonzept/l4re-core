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

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <l4/atkins/bare/tap_logger>

static Atkins::Bare::Tap_logger tap;

extern "C" int
accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);

/**
 * Trace the errno to help debug cases where the weakly defined noop symbol
 * is over-shadowed by another implementation.
 *
 * \param errnum  An errno error code.
 */
static void
trace_errno(int errnum)
{
  printf("errno is %d %s\n", errnum, strerror(errnum));
}

/**
 * Verify the expectation that errno has been set and is less than 0.
 *
 * \param errnum  The errno error code to verify.
 */
static void
check_errno(int errnum)
{
  tap.expect(0 < errnum, "An error is signalled via errno.");
  trace_errno(errnum);
}

/**
 * 'socket()' can be called.
 */
static void
test_socketnoop_socket()
{
  int domain = 0;
  int type = 1;
  int protocol = 2;

  errno = 0;
  tap.expect(-1 == socket(domain, type, protocol), "socket() is callable.");
  check_errno(errno);
}

/**
 * 'connect()' can be called.
 */
static void
test_socketnoop_connect()
{
  int sockfd = -2;
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);

  errno = 0;
  tap.expect(-1 == connect(sockfd, &addr, addrlen), "connect() is callable.");
  check_errno(errno);
}

/**
 * 'accept()' can be called.
 */
static void
test_socketnoop_accept()
{
  int sockfd = -2;
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);

  errno = 0;
  tap.expect(-1 == accept(sockfd, &addr, &addrlen), "accept() is callable.");
  check_errno(errno);
}

/**
 * 'accept4()' can be called.
 */
static void
test_socketnoop_accept4()
{
  int sockfd = -2;
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);
  int flags = 4;

  errno = 0;
  tap.expect(-1 == accept4(sockfd, &addr, &addrlen, flags),
             "accept4() is callable.");
  check_errno(errno);
}

/**
 * 'bind()' can be called.
 */
static void
test_socketnoop_bind()
{
  int sockfd = -2;
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);

  errno = 0;
  tap.expect(-1 == bind(sockfd, &addr, addrlen), "bind() is callable.");
  check_errno(errno);
}

/**
 * 'getsockopt()' can be called.
 */
static void
test_socketnoop_getsockopt()
{
  int sockfd = -2;
  int level = 5;
  int optname = 6;
  int optval = 0;
  socklen_t optlen = sizeof(optval);

  errno = 0;
  tap.expect(-1 == getsockopt(sockfd, level, optname, &optval, &optlen),
             "getsockopt() is callable.");
  check_errno(errno);
}

/**
 * 'setsockopt()' can be called.
 */
static void
test_socketnoop_setsockopt()
{
  int sockfd = -2;
  int level = 5;
  int optname = 6;
  int optval = 0;
  socklen_t optlen = sizeof(optval);

  errno = 0;
  tap.expect(-1 == setsockopt(sockfd, level, optname, &optval, optlen),
             "setsockopt() is callable.");
  check_errno(errno);
}

/**
 * 'listen()' can be called.
 */
static void
test_socketnoop_listen()
{
  int sockfd = -2;
  int backlog = 7;

  errno = 0;
  tap.expect(-1 == listen(sockfd, backlog), "listen() is callable.");
  check_errno(errno);
}

/**
 * 'shutdown()' can be called.
 */
static void
test_socketnoop_shutdown()
{
  int sockfd = -2;
  int how = 8;

  errno = 0;
  tap.expect(-1 == shutdown(sockfd, how), "shutdown() is callable.");
  check_errno(errno);
}

/**
 * 'recv()' can be called.
 */
static void
test_socketnoop_recv()
{
  int sockfd = -2;
  char buf[10];
  size_t buflen = sizeof(buf);
  int flags = 4;

  errno = 0;
  tap.expect(-1 == recv(sockfd, buf, buflen, flags), "recv() is callable.");
  check_errno(errno);
}

/**
 * 'recvfrom()' can be called.
 */
static void
test_socketnoop_recvfrom()
{
  int sockfd = -2;
  char buf[10];
  size_t buflen = sizeof(buf);
  int flags = 4;
  struct sockaddr src_addr;
  socklen_t addrlen = sizeof(src_addr);

  errno = 0;
  tap.expect(-1 == recvfrom(sockfd, buf, buflen, flags, &src_addr, &addrlen),
             "recvfrom() is callable.");
  check_errno(errno);
}

/**
 * 'send()' can be called.
 */
static void
test_socketnoop_send()
{
  int sockfd = -2;
  char buf[10];
  size_t buflen = sizeof(buf);
  int flags = 4;

  errno = 0;
  tap.expect(-1 == send(sockfd, buf, buflen, flags), "send() is callable.");
  check_errno(errno);
}

/**
 * 'sendto()' can be called.
 */
static void
test_socketnoop_sendto()
{
  int sockfd = -2;
  char buf[10];
  size_t buflen = sizeof(buf);
  int flags = 4;
  struct sockaddr dest_addr;
  socklen_t addrlen = sizeof(dest_addr);

  errno = 0;
  tap.expect(-1 == sendto(sockfd, buf, buflen, flags, &dest_addr, addrlen),
             "sendto() is callable.");
  check_errno(errno);
}

/**
 * 'socketpair()' can be called.
 */
static void
test_socketnoop_socketpair()
{
  int domain = 0;
  int type = 1;
  int protocol = 2;
  int sv[2];

  errno = 0;
  tap.expect(-1 == socketpair(domain, type, protocol, sv),
             "socketpair() is callable.");
  check_errno(errno);
}

/**
 * 'getsockname()' can be called.
 */
static void
test_socketnoop_getsockname()
{
  int sockfd = -2;
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);

  errno = 0;
  tap.expect(-1 == getsockname(sockfd, &addr, &addrlen),
             "getsockname() is callable.");
  check_errno(errno);
}

/**
 * 'getpeername()' can be called.
 */
static void
test_socketnoop_getpeername()
{
  int sockfd = -2;
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);

  errno = 0;
  tap.expect(-1 == getpeername(sockfd, &addr, &addrlen),
             "getpeername() is callable.");
  check_errno(errno);
}

/**
 * 'getnameinfo()' can be called.
 */
static void
test_socketnoop_getnameinfo()
{
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);
  char host[4];
  socklen_t hostlen = sizeof(host);
  char serv[4];
  socklen_t servlen = sizeof(serv);
  int flags = 4;

  // errno is only set on EAI_SYSTEM
  tap.expect(EAI_FAIL
               == getnameinfo(&addr, addrlen, host, hostlen, serv, servlen,
                              flags),
             "getnameinfo() is callable.");
}

/**
 * 'gethostent_r()' can be called.
 */
static void
test_socketnoop_gethostent_r()
{
  struct hostent ret;
  char buf[10];
  size_t buflen = sizeof(buf);
  struct hostent *result = reinterpret_cast<struct hostent *>(-1);
  int h_errnop = 0;

  tap.expect(0 != gethostent_r(&ret, buf, buflen, &result, &h_errnop),
             "gethostent_r() is callable.");
  tap.expect(NULL == result, "Result pointer is NULL in case of error.");
  tap.expect(0 < h_errnop,
             "An error is signalled via passed in pointer to h_errnop.");
  trace_errno(h_errnop);
}

int
main(int, char **)
{
  tap.start();

  test_socketnoop_accept();
  test_socketnoop_accept4();
  test_socketnoop_bind();
  test_socketnoop_connect();
  test_socketnoop_gethostent_r();
  test_socketnoop_getnameinfo();
  test_socketnoop_getpeername();
  test_socketnoop_getsockname();
  test_socketnoop_getsockopt();
  test_socketnoop_listen();
  test_socketnoop_recv();
  test_socketnoop_recvfrom();
  test_socketnoop_send();
  test_socketnoop_sendto();
  test_socketnoop_setsockopt();
  test_socketnoop_shutdown();
  test_socketnoop_socket();
  test_socketnoop_socketpair();

  return tap.finish();
}
