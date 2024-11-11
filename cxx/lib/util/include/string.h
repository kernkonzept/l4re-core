/**
 * \file
 * \brief String
 */
/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/cxx/basic_ostream>

namespace L4 {

  /**
   * \brief A null-terminated string container class.
   * \ingroup cxx_api
   */
  class String
  {
  public:
    String(char const *str = "") : _str(str)
    {}

    unsigned length() const
    {
      unsigned l;
      for (l = 0; _str[l]; l++)
        ;
      return l;
    }

    char const *p_str() const { return _str; }

  private:
    char const *_str;
  };
}

inline
L4::BasicOStream &operator << (L4::BasicOStream &o, L4::String const &s)
{
  o << s.p_str();
  return o;
}
