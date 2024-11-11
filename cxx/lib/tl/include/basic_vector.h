/**
 * \file
 * \brief Basic vector
 */
/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
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
