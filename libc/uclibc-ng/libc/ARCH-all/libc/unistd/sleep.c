#include <time.h>
#include <unistd.h>

unsigned int sleep (unsigned int seconds)
{
  unsigned int res;
  struct timespec ts = { .tv_sec = (long int) seconds, .tv_nsec = 0 };
  res = nanosleep(&ts, &ts);
  if (res) res = (unsigned int) ts.tv_sec + (ts.tv_nsec >= 500000000L);
  return res;
}

