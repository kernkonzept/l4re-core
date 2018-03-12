/* Copyright (C) 2013 Gentoo Foundation
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <stdarg.h>
#include <obstack.h>

int
obstack_vprintf (struct obstack *obstack, const char *format, va_list args)
{
  int n;
  char *s;
  n = vasprintf(&s, format, args);
  obstack_grow(obstack, s, n);
  return n;
}
libc_hidden_def(obstack_vprintf)

int
obstack_printf (struct obstack *obstack, const char *format, ...)
{
  int n;
  va_list ap;
  va_start (ap, format);
  n = obstack_vprintf (obstack, format, ap);
  va_end (ap);
  return n;
}
