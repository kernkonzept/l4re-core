/**
 * \file
 * \brief String
 */
/*
 * (c) 2004-2009 Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
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

#ifndef L4_CXX_STRING_H__
#define L4_CXX_STRING_H__

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

};

inline
L4::BasicOStream &operator << (L4::BasicOStream &o, L4::String const &s)
{
  o << s.p_str();
  return o;
}

#endif /* L4_CXX_STRING_H__ */
