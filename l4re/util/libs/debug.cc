/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/util/debug>

#include <stdarg.h>
#include <stdio.h>

namespace L4Re { namespace Util {

static FILE *&out = stdout;
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

# ifdef __clang__

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

# else

int
Dbg::printf_impl(char const *fmt, ...) const
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
Dbg::cprintf_impl(char const *fmt, ...) const
{
  int n;
  va_list args;

  va_start    (args, fmt);
  n = vfprintf (out, fmt, args);
  va_end      (args);

  return n;
}

# endif

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
