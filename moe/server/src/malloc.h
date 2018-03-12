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
  class Guarded_alloc
  {
  public:
    Guarded_alloc(Malloc_container* c, size_t sz, size_t align)
    : _c(c)
    {
      _p = c->alloc(sz, align);
      if (L4_UNLIKELY(!_p))
        throw L4::Out_of_memory();
    }

    ~Guarded_alloc()
    {
      if (_p)
        _c->free(_p);
    }

    template <typename T>
    T *release()
    {
      T *ret = static_cast<T *>(_p);
      _p = 0;
      return ret;
    }

    void *get() const
    { return _p; }

  private:
    void *_p;
    Malloc_container* _c;
  };

  void *alloc(size_t size, size_t align) throw();
  void free(void *block) throw();
  virtual void reparent(Malloc_container *new_container);

  template <typename T, typename ...ARGS>
  T *make_obj(ARGS &&... args)
  {
    Guarded_alloc q(this, sizeof(T), alignof(T));

    new (q.get()) T(cxx::forward<ARGS>(args)...);

    return q.release<T>();
  }

  template <typename T>
  T *alloc(size_t size)
  {
    Guarded_alloc q(this, sizeof(T) * size, alignof(T));

    new (q.get()) T[size];

    return q.release<T>();
  }

  static Malloc_container *from_ptr(void const *p) throw();

protected:
  virtual void *get_mem();
  virtual void free_mem(void *page);

private:
  cxx::S_list<Malloc_page> _pages;
};

} // namespace
