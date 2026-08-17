/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/cxx/exceptions>

#include <cstddef>

class Dbg;

/**
 * Base page allocator.
 *
 * Manages the physical memory pages available to Moe.
 */
class Single_page_alloc_base
{
public:
  enum Nothrow { nothrow };

  struct Config
  {
    l4_addr_t physmin;
    l4_addr_t physmax;

    constexpr Config() : physmin(0), physmax(~0UL) {}

    explicit Config(l4_addr_t physmin, l4_addr_t physmax)
    : physmin(physmin), physmax(physmax) {}
  };

  static Config default_mem_cfg;

protected:
  Single_page_alloc_base();

public:
  static void *_alloc_max(size_t min, size_t *max, size_t align,
                          size_t granularity, Config cfg);
  static void *_alloc(Nothrow, size_t size, size_t align,
                      Config cfg = default_mem_cfg);
  static void *_alloc(size_t size, size_t align, Config cfg = default_mem_cfg)
  {
    void *r = _alloc(nothrow, size, align, cfg);
    if (!r)
      throw L4::Out_of_memory();
    return r;
  }
  static void _free(void *p, size_t size);
  static size_t _avail();

  static void _add_mem(void *p, size_t size);

#ifndef NDEBUG
  static void _dump_free(Dbg &dbg);
#endif

  static bool can_free;
};

class Single_page_unique_ptr
{
private:
  void *_p = 0;
  size_t _s;

public:
  void *release()
  {
    void *p = _p;
    _p = 0;
    return p;
  }

  void reset(void *n = 0, size_t size = 0)
  {
    if (n == _p)
      return;

    void *p = _p;
    size_t s = _s;

    _p = n;
    if (n)
      _s = size;

    if (p)
      Single_page_alloc_base::_free(p, s);
  }

  size_t size() const { return _s; }

  void *get() const { return _p; }
  void *operator * () const { return _p; }

  Single_page_unique_ptr() = default;
  Single_page_unique_ptr(void *p, size_t size) : _p(p), _s(size) {}

  ~Single_page_unique_ptr()
  { reset(); }

  Single_page_unique_ptr &operator = (Single_page_unique_ptr &&o)
  {
    if (this == &o)
      return *this;

    reset(o._p, o._s);
    o._p = 0;
    return *this;
  }
};
