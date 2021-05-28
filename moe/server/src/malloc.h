/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/consts.h>
#include <l4/cxx/exceptions>
#include <l4/cxx/type_traits>
#include <l4/cxx/slist>

#include <new>
#include <cstddef>

namespace Moe {

class Malloc_page;

/**
 * A basic allocator for any size of chunks.
 */
class Malloc_container
{
public:
  void *alloc(size_t size, size_t align) throw();
  void free(void *block) throw();
  virtual void reparent(Malloc_container *new_container);

  template <typename T, typename ...ARGS>
  T *make_obj(ARGS &&... args)
  {
    void *obj = alloc(sizeof(T), alignof(T));
    if (obj)
      new (obj) T(cxx::forward<ARGS>(args)...);

    return reinterpret_cast<T*>(obj);
  }

  template <typename T>
  T *alloc(size_t size)
  {
    // FIXME: check integer overflow
    void *arr = alloc(sizeof(T) * size, alignof(T));
    if (arr)
      new (arr) T[size];

    return reinterpret_cast<T*>(arr);
  }

  static Malloc_container *from_ptr(void const *p) throw();

protected:
  virtual void *get_mem();
  virtual void free_mem(void *page);

private:
  cxx::S_list<Malloc_page> _pages;
};

} // namespace
