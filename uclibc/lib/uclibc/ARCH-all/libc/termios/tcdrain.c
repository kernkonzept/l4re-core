
#include <termios.h>
#include <stdio.h>
#include <errno.h>

int tcdrain(int fd)
{
  printf("tcdrain: not implemented\n");
  errno = ENOTTY;
  return -1;
}
