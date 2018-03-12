/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
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
#include <l4/re/util/debug>

#include <stdarg.h>
#include <stdio.h>

namespace L4Re { namespace Util {

static FILE *out = stdout;
#ifndef NDEBUG
unsigned long Dbg::level = 1;

void
Dbg::tag() const
{
  if (!_component)
    return;
  if (_subsys)
    cprintf("%s[%s]: ", _component, _subsys);
  else
    cprintf("%s: ", _component);
}


int
Dbg::printf(char const *fmt, ...) const
{
  if (!(level & _m))
    return 0;

  tag();

  int n;
  va_list args;

  va_start    (args, fmt);
  n = vfprintf (out, fmt, args);
  va_end      (args);

  return n;
}

int
Dbg::cprintf(char const *fmt, ...) const
{
  if (!(level & _m))
    return 0;

  int n;
  va_list args;

  va_start    (args, fmt);
  n = vfprintf (out, fmt, args);
  va_end      (args);

  return n;
}
#endif /* NDEBUG */


char const *const Err::levels[] =
{ "ERROR: ", "FATAL: " };

int
Err::printf(char const *fmt, ...) const
{
  tag();

  int n;
  va_list args;

  va_start    (args, fmt);
  n = vfprintf (out, fmt, args);
  va_end      (args);

  return n;
}

int
Err::cprintf(char const *fmt, ...) const
{
  int n;
  va_list args;

  va_start    (args, fmt);
  n = vfprintf (out, fmt, args);
  va_end      (args);

  return n;
}

}}
