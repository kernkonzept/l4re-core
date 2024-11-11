/*!
 * \file   l4util/lib/src/kprintf.c
 * \brief  simple(!) -- non threaded -- printf using kernel debugger output
 *
 * \date   04/05/2007
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2007-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#include <stdio.h>
#include <stdarg.h>

#include <l4/sys/kdebug.h>
#include <l4/util/kprintf.h>

/* This is in the BSS on purpose to not put more load on the stack,
 * and we know that this is suited for threading this way
 */
static char buffer[500];

L4_CV int l4_kprintf(const char *fmt, ...)
{
  va_list list;
  int err;

  va_start(list, fmt);
  err = vsnprintf(buffer, sizeof(buffer), fmt, list);
  va_end(list);

  outstring(buffer);

  return err;
}
