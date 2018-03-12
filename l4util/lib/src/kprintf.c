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
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
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
  buffer[sizeof(buffer) - 1] = 0;
  va_end(list);

  outstring(buffer);

  return err;
}
