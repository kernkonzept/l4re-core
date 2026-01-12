/*
 * (c) 2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/*
 * Copyright (C) 2014-2015, 2018-2020, 2023-2025 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 */
#pragma once

#include <l4/cxx/exceptions>

#include <new>
#include <cstddef>
#include <cstdio>
#include <cassert>

#include "malloc.h"
#include "page_alloc.h"

namespace Moe {

/**
 * A simple quota manager.
 */
class Quota
{
public:
  /**
   * Create new quota.
   *
   * The quota is reserved at the optional parent. If the reservation cannot
   * be fulfilled, the constructor will throw an Out_of_memory exception.
   */
  explicit Quota(Quota *parent, size_t limit)
  : _parent(parent), _limit(limit)
  {
    // Unlimited quotas as child of a limited quota are not allowed.
    assert(!(parent && parent->_limit && !limit));
    if (parent && !parent->reserve(limit))
      throw L4::Out_of_memory();
  }

  ~Quota()
  {
    assert(!_reserved && !_committed);
    if (_parent)
      _parent->relinquish(_limit);
  }

  /**
   * Allocate from quota.
   *
   * Allocations are backed by actual memory pages and are counted towards
   * committed(). They are also accounted to the reserved quota and propagated
   * to parent quotas.
   */
  bool alloc(size_t s)
  {
    if (!reserve(s))
      return false;

    for (Quota *q = this; q; q = q->_parent)
      if (q->_limit)
        q->_committed += s;

    return true;
  }

  /**
   * Free allocation from quota.
   *
   * The inverse operation of alloc().
   */
  void free(size_t s)
  {
    relinquish(s);

    for (Quota *q = this; q; q = q->_parent)
      if (q->_limit)
        q->_committed -= s;
  }

  /** Quota limit */
  size_t limit() const { return _limit; }
  /** Amount of reserved quota */
  size_t reserved() const { return _reserved; }
  /** Amount of actually committed memory */
  size_t committed() const { return _committed; }

private:
  /**
   * Reserve portion of quota for sub-quotas and allocations.
   *
   * Reservations are not backed by actual pages. They are made to ensure that
   * future allocations will succeed. As such, they are only accounted in the
   * object itself. Of course, this requires that this quota is backed by
   * existing memory and the system is not over committed.
   */
  bool reserve(size_t s)
  {
    if (!_limit)
      return true;

    if (s > _limit || _reserved > _limit - s)
      return false;

    _reserved += s;
    return true;
  }

  /**
   * Release reservation.
   *
   * The inverse operation of reserve().
   */
  void relinquish(size_t s)
  {
    if (!_limit)
      return;

    assert(s <= _reserved);
    _reserved -= s;
  }

  Quota *_parent;
  size_t _limit;
  size_t _reserved = 0;
  size_t _committed = 0;
};

/**
 * Quota allocator that cleans up on unexpected exit.
 */
struct Quota_guard
{
  Quota *q;
  size_t amount;

  Quota_guard() : q(0), amount(0) {}

  Quota_guard(Quota *q, size_t amount) : q(q), amount(amount)
  {
    if (!q->alloc(amount))
      throw L4::Out_of_memory();
  }

  ~Quota_guard()
  {
    if (q)
      q->free(amount);
  }

  void release()
  {
    q = 0;
  }

  Quota_guard &operator = (Quota_guard &&g)
  {
    if (this == &g)
      return *this;

    if (q)
      q->free(amount);

    q = g.q;
    if (q)
      amount = g.amount;

    g.release();

    return *this;
  }

  template< typename T >
  T release(T t)
  {
    q = 0;
    return t;
  }
};

/**
 * Quota-guarded allocator.
 */
class Q_alloc : public Malloc_container
{
public:
  Q_alloc(Quota *parent, size_t limit) : _quota(parent, limit) {}

  Quota *quota() { return &_quota; }

  void *alloc_pages(unsigned long size, unsigned long align,
                    Single_page_alloc_base::Config cfg)
  {
    Quota_guard g(quota(), size);
    return g.release(Single_page_alloc_base::_alloc(size, align, cfg));
  }

  void free_pages(void *p, unsigned long size) noexcept
  {
    Single_page_alloc_base::_free(p, size);
    quota()->free(size);
  }

protected:
  void *get_mem() override;
  void free_mem(void *page) override;

  Quota _quota;
};

/**
 * An object that is saved in quota storage.
 */

class Q_object
{
public:
  Q_alloc *qalloc() const
  {
    return static_cast<Q_alloc *>(Malloc_container::from_ptr(this));
  }
};


// The static allocator for global memory
struct Moe_alloc : public Q_alloc
{
public:
  static Moe_alloc *allocator();

private:
  Moe_alloc() : Q_alloc(nullptr, 0) {}
};


/**
 * An allocator that gets memory from the Q_alloc responsible for
 * the memory where this allocator is placed.
 */
template <typename T>
class Quota_allocator
{
public:
  enum { can_free = true };

  Quota_allocator() noexcept {}
  Quota_allocator(Quota_allocator const &) noexcept {}
  Quota &operator = (Quota const &) = delete;

  ~Quota_allocator() noexcept {}

  T *alloc() noexcept
  {
    auto *mc = Malloc_container::from_ptr(this);
    return static_cast<T *>(mc->alloc(sizeof (T), alignof(T)));
  }

  void free(T *t) noexcept
  {
    auto *mc = Malloc_container::from_ptr(t);
    mc->free(t);
  }
};

} // namespace

