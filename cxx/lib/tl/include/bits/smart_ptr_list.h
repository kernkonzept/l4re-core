/**
 * \file Implementation of a list of smart-pointer-managed objects.
 */
/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include "../type_traits"

namespace cxx { namespace Bits {

/**
 * List item for an arbitrary item in a Smart_ptr_list.
 *
 * \tparam T        Type of object to be stored in the list.
 * \tparam STORE_T  Storage type for pointer to next item. The class must
 *                  implement a get() function that returns a pointer to the
 *                  stored object and destroy the stored object when the item
 *                  goes out of scope.
 */
template <typename T, typename STORE_T>
class Smart_ptr_list_item
{
  using Value_type = T;
  using Storage_type = STORE_T;

  template<typename U> friend class Smart_ptr_list;
  Storage_type _n;
};

/**
 * List of smart-pointer-managed objects.
 *
 * \tparam ITEM  Type of the list items.
 *
 * The list is implemented as a single-linked list connected via smart pointers,
 * so that they are automatically cleaned up when they are removed from the
 * list.
 */
template <typename ITEM>
class Smart_ptr_list
{
  using Value_type = typename ITEM::Value_type;
  using Next_type = typename ITEM::Storage_type;

public:
  class Iterator
  {
  public:
    Iterator() : _c(nullptr) {}

    Value_type *operator * () const { return _c; }
    Value_type *operator -> () const { return _c; }

    Iterator operator ++ ()
    {
      _c = _c->_n.get();
      return *this;
    }

    bool operator == (Iterator const &o) const { return _c == o._c; }
    bool operator != (Iterator const &o) const { return !operator == (o); }

  private:
    friend class Smart_ptr_list;

    explicit Iterator(Value_type *i) : _c(i) {}

    Value_type *_c;
  };

  class Const_iterator
  {
  public:
    Const_iterator() : _c(nullptr) {}

    Value_type const *operator * () const { return _c; }
    Value_type const *operator -> () const { return _c; }

    Const_iterator operator ++ ()
    {
      _c = _c->_n.get();
      return *this;
    }

    bool operator == (Const_iterator const &o) const { return _c == o._c; }
    bool operator != (Const_iterator const &o) const { return !operator == (o); }

  private:
    friend class Smart_ptr_list;

    explicit Const_iterator(Value_type const *i) : _c(i) {}

    Value_type const *_c;
  };

  Smart_ptr_list() : _b(&_f) {}

  /// Add an element to the front of the list.
  void push_front(Next_type &&e)
  {
    e->_n = cxx::move(this->_f);
    this->_f = cxx::move(e);

    if (_b == &_f)
      _b = &(_f->_n);
  }

  /// Add an element to the front of the list.
  void push_front(Next_type const &e)
  {
    e->_n = cxx::move(this->_f);
    this->_f = e;

    if (_b == &_f)
      _b = &(_f->_n);
  }

  /// Add an element at the end of the list.
  void push_back(Next_type &&e)
  {
    *_b = cxx::move(e);
    _b = &((*_b)->_n);
  }

  /// Add an element at the end of the list.
  void push_back(Next_type const &e)
  {
    *_b = e;
    _b = &((*_b)->_n);
  }

  /// Return a pointer to the first element in the list.
  Value_type *front() const
  { return _f.get(); }

  /**
   * Remove the element in front of the list and return it.
   *
   * \return The element that was previously in front of the list
   *         as a managed pointer or a nullptr-equivalent when the
   *         list was already empty.
   */
  Next_type pop_front()
  {
    Next_type ret = cxx::move(_f);

    if (ret)
      _f = cxx::move(ret->_n);

    if (!_f)
      _b = &_f;

    return ret;
  }

  /// Check if the list is empty.
  bool empty() const
  { return !_f; }

  Iterator begin() { return Iterator(_f.get()); }
  Iterator end() { return Iterator(); }

  Const_iterator begin() const { return Const_iterator(_f.get()); }
  Const_iterator end() const { return Const_iterator(); }

  Const_iterator cbegin() const { return const_iterator(_f.get()); }
  Const_iterator cend() const { return Const_iterator(); }

private:
  Next_type _f;
  Next_type *_b;
};

} }
