/**
 * \file
 * \brief Basic vector
 */
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
#pragma once

#include <l4/cxx/std_alloc>

namespace cxx {

template< typename T >
class Basic_vector
{
public:
  Basic_vector(T *array, unsigned long capacity)
  : _array(array), _capacity(capacity)
  {
    for (unsigned long i = 0; i < capacity; ++i)
      new (&_array[i]) T();
  }

private:
  T *_array;
  unsigned long _capacity;
};

};
