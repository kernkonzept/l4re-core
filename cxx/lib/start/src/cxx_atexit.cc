/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */

#include "cxx_atexit.h"

#define NUM_ATEXIT	64

struct __exit_handler
{
  void (*f)(void *);
  void *arg;
  void *dso_handle;
};

static __exit_handler __atexitlist[NUM_ATEXIT];
static volatile unsigned atexit_counter;
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
  return __cxa_atexit((void (*)(void*))f, 0, (!&__dso_handle)?0:__dso_handle);
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

