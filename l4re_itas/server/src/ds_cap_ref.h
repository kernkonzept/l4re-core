/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/cxx/avl_map>
#include <l4/re/dataspace>

/**
 * Utility class to hold ITAS-owned capability slots for application dataspace
 * capabilities.
 *
 * Used to ensure that the dataspace capability of a region stays alive even if
 * the application unmaps its capability slot. Unfortunately, the application
 * may reuse capability slots any time without ITAS noticing. So we cannot rely
 * on the application capability slot alone beyond providing a hint.
 * Effectively, if the application capability slot matches, we have to actually
 * compare capabilies to be sure it is still the same.
 */
class Ds_cap_references
{
  template<typename Cap>
  struct Cap_cmp
  {
    bool operator()(Cap const &l, Cap const &r) const noexcept
    { return l.cap() < r.cap(); }
  };

  struct App_cap_slot
  {
    App_cap_slot() = default;
    explicit App_cap_slot(l4_cap_idx_t app_cap) : refcnt(1), app_cap(app_cap) {}

    unsigned refcnt = 0;

    /**
     * Original application cap slot that brought the App_cap_slot to life.
     *
     * Will be reset to L4_INVALID_CAP whenever we detect that the application
     * reused a cap slot.
     */
    l4_cap_idx_t app_cap = L4_INVALID_CAP;
  };

  /**
   * ITAS references to attached dataspaces.
   */
  using Cap_table = cxx::Avl_map<L4::Cap<L4Re::Dataspace>, App_cap_slot, Cap_cmp>;

  /**
   * Fast lookup from application capability index to ITAS capability mirror.
   *
   * Actually, this is a probabilistic index. As the application may reuse
   * capability slots, the capability must be compared to be sure the lookup
   * is valid.
   */
  using App_cap_index = cxx::Avl_map<l4_cap_idx_t, L4::Cap<L4Re::Dataspace>>;

public:
  /**
   * Shared reference to an application L4Re::Dataspace.
   */
  class Ds_cap_ref
  {
    friend class Ds_cap_references;

  public:
    /// Construct an invalid cap
    Ds_cap_ref() noexcept = default;

    /**
     * Create an object for an existing capability in ITAS.
     *
     * The class will not take ownership! The capability is supposed to live
     * at least as long as this object.
     */
    explicit Ds_cap_ref(L4::Cap<L4Re::Dataspace> itas_cap) noexcept
    : _itas_cap(itas_cap)
    {}

    Ds_cap_ref(Ds_cap_ref const &o) noexcept
    : _parent(o._parent), _itas_cap(o._itas_cap)
    {
      if (_parent)
        _parent->take(_itas_cap);
    }

    Ds_cap_ref(Ds_cap_ref &&o) noexcept
    : _parent(o._parent), _itas_cap(o._itas_cap)
    {
      o._parent = nullptr;
      o._itas_cap = L4::Cap<L4Re::Dataspace>();
    }

    ~Ds_cap_ref()
    {
      if (_parent)
        _parent->release(_itas_cap);
    }

    Ds_cap_ref &operator=(Ds_cap_ref const &o)
    {
      if (&o == this)
        return *this;

      reset();
      _parent = o._parent;
      _itas_cap = o._itas_cap;
      if (_parent)
        _parent->take(_itas_cap);

      return *this;
    }

    Ds_cap_ref &operator=(Ds_cap_ref &&o)
    {
      if (&o == this)
        return *this;

      reset();
      _parent = o._parent;
      _itas_cap = o._itas_cap;
      o._parent = nullptr;
      o._itas_cap = L4::Cap<L4Re::Dataspace>();

      return *this;
    }

    explicit operator bool() const noexcept
    { return _itas_cap.is_valid(); }

    L4::Cap<L4Re::Dataspace> itas_cap() const
    { return _itas_cap; }

    void reset() noexcept
    {
      if (_parent)
        {
          _parent->release(_itas_cap);
          _parent = nullptr;
        }
      _itas_cap = L4::Cap<L4Re::Dataspace>();
    }

  private:
    Ds_cap_ref(Ds_cap_references *parent, L4::Cap<L4Re::Dataspace> itas_cap)
    : _parent(parent), _itas_cap(itas_cap)
    {}

    Ds_cap_references *_parent = nullptr;
    L4::Cap<L4Re::Dataspace> _itas_cap;
  };

  Ds_cap_ref lookup(l4_cap_idx_t app_cap);

protected:
  void take(L4::Cap<L4Re::Dataspace> itas_cap);
  void release(L4::Cap<L4Re::Dataspace> itas_cap);

private:
  Cap_table _caps;
  App_cap_index _app_cap_index;
};
