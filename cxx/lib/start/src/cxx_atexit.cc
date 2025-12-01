/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <atomic>

#include "cxx_atexit.h"

#define NUM_ATEXIT	64

struct __exit_handler
{
  void (*f)(void *);
  void *arg;
  void *dso_handle;
};

static __exit_handler __atexitlist[NUM_ATEXIT];
static std::atomic<unsigned> atexit_counter;
int __cxa_atexit(void (*f)(void*), void *arg, void *dso_handle)
{
  unsigned c = atexit_counter++;
  if (c >= NUM_ATEXIT)
    return -1;

  __atexitlist[c].f = f;
  __atexitlist[c].arg = arg;
  __atexitlist[c].dso_handle = dso_handle;

  return 0;
}

extern void *__dso_handle __attribute__((weak));

int atexit(void (*f)(void))
{
  void (*fct)(void *) = reinterpret_cast<void (*)(void*)>(f);
  return __cxa_atexit(fct, 0, (!&__dso_handle) ? nullptr : __dso_handle);
}

void __cxa_finalize(void *dso_handle)
{
  unsigned co = atexit_counter;
  if (co > NUM_ATEXIT)
    co = NUM_ATEXIT;

  while(co)
    {
      __exit_handler *h = &__atexitlist[--co];
      if (h->f && (dso_handle == 0 || h->dso_handle == dso_handle))
        {
          h->f(h->arg);
          h->f = 0;
        }
    }
}
