/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/re/util/debug>

class Err : public L4Re::Util::Err
{
public:
  explicit
  Err(Level l = Normal) : L4Re::Util::Err(l, "L4Re") {}
};

class Dbg : public L4Re::Util::Dbg
{
public:
  enum Level
  {
    Info       = 1,
    Warn       = 2,
    Boot       = 4,

    Server     = 0x10,
    Exceptions = 0x20,
    POSIX_be   = 0x40,
    Loader     = 0x80,
  };

  explicit
  Dbg(unsigned long mask, char const *subs = 0)
  : L4Re::Util::Dbg(mask, "L4Re", subs)
  {}
};

