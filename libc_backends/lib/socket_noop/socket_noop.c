/*
 * (c) 2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

/* Define all functions weak so that real implementation can overwrite them */
#define W __attribute__((weak))

int W socket(int domain, int type, int protocol)
{
  printf("Unimplemented: %s(%d, %d, %d)\n", __func__, domain, type, protocol);
  errno = EINVAL;
  return -1;
}

int W socketpair(int domain, int type, int protocol, int sv[2])
{
  printf("Unimplemented: %s(%d, %d, %d, %p)\n", __func__,
         domain, type, protocol, sv);
  errno = EOPNOTSUPP;
  return -1;
}

int W getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, unsigned int flags)
{
  printf("Unimplemented: %s(%p, %d, %p, %d, %p, %d, %d)\n", __func__,
         sa, salen, host, hostlen, serv, servlen, flags);
  return EAI_FAIL;
}
