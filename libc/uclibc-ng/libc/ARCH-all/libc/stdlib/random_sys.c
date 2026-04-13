
#include <sys/random.h>
#include <errno.h>

int getrandom(void *__buf, size_t count, unsigned int flags)
{
  /* Could implement using L4Re::random here */
  return -ENODEV;
}
