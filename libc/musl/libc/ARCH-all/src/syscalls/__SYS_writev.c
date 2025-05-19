#include <sys/uio.h>
#include <unistd.h>
#include <syscall.h>

ssize_t __l4re_syscall_SYS_writev(int fd, const struct iovec *iov, int iovcnt)
{
  ssize_t written = 0;
  for (int i = 0; i < iovcnt; ++i)
    {
      if (write(fd, iov[i].iov_base, iov[i].iov_len) == -1)
        return -1;
      written += iov[i].iov_len;
    }

  return written;
}

