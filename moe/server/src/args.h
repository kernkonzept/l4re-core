/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/string>
#include <l4/cxx/pair>

#include <cctype>

static
cxx::Pair<cxx::String, cxx::String> next_arg(cxx::String const &cmdline)
{
  cxx::String cp = cmdline;
  char quote = 0;

  //char const *arg = 0;

  while (!cp.empty() && isspace(cp[0]))
    cp = cp.substr(1);

  if (cp.empty())
    return cxx::pair(cp, cp);

  if (cp[0] == '"' || cp[0] == '\'')
    {
      quote = cp[0];
      cp = cp.substr(1);
    }

  cxx::String::Index e;
  if (quote)
    e = cp.find(quote);
  else
    e = cp.find_match([](int c) { return isspace(c); });

  // missing end quote
  if (quote && cp.eof(e))
    return cxx::pair(cxx::String(), cxx::String());

  return cxx::pair(cp.head(e), cp.substr(e+1));
}

