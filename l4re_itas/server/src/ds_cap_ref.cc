/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <assert.h>
#include <stdlib.h>

#include "ds_cap_ref.h"
#include "globals.h"

inline void *operator new (size_t s, cxx::Nothrow const &) noexcept
{ return malloc(s); }

inline void operator delete (void *p, cxx::Nothrow const &) noexcept
{ return free(p); }

void Ds_cap_references::take(L4::Cap<L4Re::Dataspace> itas_cap)
{
  auto slot = _caps.find_node(itas_cap);
  assert(slot);

  ++const_cast<App_cap_slot &>(slot->second).refcnt;
}

void Ds_cap_references::release(L4::Cap<L4Re::Dataspace> itas_cap)
{
  auto slot = _caps.find_node(itas_cap);
  assert(slot);

  auto &cs = const_cast<App_cap_slot &>(slot->second);
  if (--cs.refcnt == 0)
    {
      Global::cap_alloc->free(itas_cap, L4Re::This_task);
      if (cs.app_cap != L4_INVALID_CAP)
        _app_cap_index.remove(cs.app_cap);
      _caps.remove(itas_cap);
    }
}

Ds_cap_references::Ds_cap_ref Ds_cap_references::lookup(l4_cap_idx_t app_cap)
{
  L4::Cap<L4Re::Dataspace> itas_cap;

  // Maybe we know this capability already...
  if (auto it = _app_cap_index.find(app_cap); it != _app_cap_index.end())
    itas_cap = it->second;

  if (itas_cap)
    {
      // We've seen this capability slot before. Check if it still points to
      // the same object.
      if (itas_cap.is_equal(L4::Cap<L4Re::Dataspace>(app_cap)))
        {
          take(itas_cap);
          return Ds_cap_ref(this, itas_cap);
        }

      // The application reused the capability slot. Clear the index and mark
      // as invalid in our _caps table.
      _app_cap_index.remove(app_cap);
      _caps[itas_cap].app_cap = L4_INVALID_CAP;
    }

  itas_cap = Global::cap_alloc->alloc<L4Re::Dataspace>();
  if (!itas_cap)
    return Ds_cap_ref();

  if (!itas_cap.copy(L4::Cap<L4Re::Dataspace>(app_cap)))
    {
      Global::cap_alloc->free(itas_cap); // Copy failed. No need to unmap.
      return Ds_cap_ref();
    }

  auto c = _caps.insert(itas_cap, App_cap_slot(app_cap));
  if (c.second != 0)
    {
      Global::cap_alloc->free(itas_cap, L4Re::This_task);
      return Ds_cap_ref();
    }

  auto i = _app_cap_index.insert(app_cap, itas_cap);
  if (i.second != 0)
    {
      // Not enough memory (-E_exist cannot happen) to record application app
      // index. Luckily, this is just a performance problem.
      c.first->second.app_cap = L4_INVALID_CAP;
    }

  return Ds_cap_ref(this, itas_cap);
}
