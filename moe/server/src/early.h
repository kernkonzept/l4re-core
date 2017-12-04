/*
 * (c) 2017 Adam Lackorzynski <adam@l4re.org>
 *
 * This file is part of L4Re and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/vcon.h>

#include <stdlib.h>
#include <string.h>

// Very simple print, used for early (error) reporting
inline void vcon_print(char const *text)
{
  L4::Cap<L4::Vcon>(L4_BASE_LOG_CAP)->write(text, strlen(text));
}

inline long early_chksys(l4_msgtag_t const &t, char const *text)
{
  if (t.has_error() || t.label() < 0)
    {
      vcon_print(text);
      exit(1);
    }

  return t.label();
}

template<typename T>
inline
T early_chkcap(T cap, char const *text)
{
  if (!cap)
    {
      vcon_print(text);
      exit(1);
    }

  return cap;
}
