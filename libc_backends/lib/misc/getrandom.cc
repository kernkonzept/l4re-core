/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/**
 * \file
 * getrandom() implementation for L4Re.
 *
 * Two sources are tried, in this order:
 *
 * 1. An L4Re::Random service is available, provided under the "rng" cap.
 * 2. The CPU's own generator, if an architecture provides randomness
 *    in user space.
 *
 * If none is available, the call fails.
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/random.h>
#include <unistd.h>

#include <l4/cxx/minmax>
#include <l4/re/env>
#include <l4/re/random>
#include <l4/sys/cxx/ipc_types>

namespace {

L4::Cap<L4Re::Random> rng;
bool rng_queried;

bool rng_service_available()
{
  if (!rng_queried)
    {
      rng_queried = true;
      rng = L4Re::Env::env()->get_cap<L4Re::Random>("rng");
    }

  return rng.is_valid();
}

/* Assumes rng_service_available() == true */
ssize_t rng_service_read(char *buf, size_t len)
{
  if (!rng_service_available())
    return -1;

  size_t done = 0;

  while (done < len)
    {
      /* Smaller than UTCB's capacity. */
      char tmp[128];
      size_t chunk = cxx::min(len - done, sizeof(tmp));

      L4::Ipc::Array<char, unsigned long> data(sizeof(tmp), tmp);
      long got = rng->get_random(chunk, &data);

      if (got < 0)
        return done ? static_cast<ssize_t>(done) : -1;

      if (got == 0)
        break;                  /* temporarily out of entropy */

      memcpy(buf + done, data.data, got);
      done += got;
    }

  return done;
}

#if defined(__x86_64__) || defined(__i386__)

bool cpu_has_rdrand()
{
  unsigned a, b, c, d;

  __asm__ __volatile__("cpuid"
                       : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
                       : "a"(1), "c"(0));

  return (c & (1u << 30)) != 0;   /* CPUID.01H:ECX.RDRAND */
}

bool rdrand_word(unsigned long *v)
{
  bool ok = false;

  /*
   * RDRAND can fail when the hardware generator is momentarily out of
   * entropy; Intel's guidance is to retry a bounded number of times.
   */
  for (int i = 0; !ok && i < 10; ++i)
    asm volatile("rdrand %0"
                 : "=r"(*v), "=@ccc"(ok)
                 :
                 : "cc");

  return ok;
}

ssize_t cpu_read(char *buf, size_t len)
{
  static int rdrand_state;          /* 0 unknown, 1 present, -1 absent */
  size_t done = 0;

  if (rdrand_state == 0)
    rdrand_state = cpu_has_rdrand() ? 1 : -1;

  if (rdrand_state < 0)
    return -1;

  while (done < len)
    {
      unsigned long v;

      if (!rdrand_word(&v))
        return done ? static_cast<ssize_t>(done) : -1;

      size_t chunk = cxx::min(sizeof(v), len - done);

      memcpy(buf + done, &v, chunk);
      done += chunk;
    }

  return done;
}

#else

ssize_t cpu_read(char *, size_t)
{ return -1; }

#endif

} // namespace

ssize_t getrandom(void *buf, size_t buflen, unsigned int flags)
noexcept(noexcept(getrandom(buf, buflen, flags)))
{
  if (flags & ~(GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE))
    {
      errno = EINVAL;
      return -1;
    }

  if (buflen == 0)
    return 0;

  char *out = static_cast<char *>(buf);

  if (rng_service_available())
    return rng_service_read(out, buflen);

  ssize_t got = cpu_read(out, buflen);
  if (got > 0)
    return got;

  errno = ENOSYS;
  return -1;
}
