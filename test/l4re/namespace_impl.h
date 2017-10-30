/*
 * Copyright (C) 2015 Kernkonzept GmbH.
 * Author(s): Sarah Hoffmann <sarah.hoffmann@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2.  Please see the COPYING-GPL-2 file for details.
 */

#include <cstdlib>
#include <cstring>
#include <l4/sys/ipc_gate>
#include <l4/sys/cxx/ipc_epiface>
#include <l4/re/util/name_space_svr>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/util/object_registry>
#include <l4/atkins/debug>

static Atkins::Dbg dbg{2, "NameSvr"};
static Atkins::Err err;


namespace Names { using namespace L4Re::Util::Names; }

class Entry : public Names::Entry
{
public:
  Entry(Names::Name const &n, Names::Obj const &o, bool dyn = false)
  : Names::Entry(n, o, dyn) {}

  void *operator new (size_t s) { return malloc(s); }
  void operator delete(void *p) { free(p); }
};

/*
 * The most simple implementation of name_space_svr using
 * the standard cap_alloc mechanism.
 *
 * Local caps can be used if they point to an Epiface. For
 * other local caps the implementation does not provide the
 * right mechanism to mark them as local (and therefore not
 * deletable).
 */
class Name_space :
  public L4::Epiface_t<Name_space, L4Re::Namespace>,
  public Names::Name_space
{
public:
  Name_space()
  : Names::Name_space(dbg, err)
  {}

  virtual ~Name_space() = default;

  Names::Entry *alloc_dynamic_entry(Names::Name const &n, unsigned flags)
  {
    char *namecpy = (char *) malloc(n.len());
    memcpy(namecpy, n.name(), n.len());
    return new Entry(Names::Name(namecpy, n.len()), Names::Obj(flags), true);
  }

  void free_dynamic_entry(Names::Entry *e)
  {
    if (e->is_dynamic())
      delete static_cast<Entry *>(e);
  }

  int get_epiface(l4_umword_t data, bool is_local, L4::Epiface **lo) override
  {
    l4_umword_t label = data;
    if (is_local)
      {
        auto c = L4::Cap<L4::Ipc_gate>(data & ~0xff);
        if (c.validate(L4_BASE_TASK_CAP).label() == 0)
          return -L4_ENODEV;
        // Note: only works with caps we own as server caps which happens to
        // be true in all our tests.
        if (l4_error(c->get_infos(&label)) != L4_EOK)
          return -L4_EINVAL;
      }

    *lo = registry->find(label);

    if (*lo)
      ++reserve_epiface_count;

    return *lo ? L4_EOK : -L4_EINVAL;
  }

  int copy_receive_cap(L4::Cap<void> *cap) override
  {
    *cap = L4::Epiface::server_iface()->rcv_cap<void>(0);

    L4::Epiface::server_iface()->realloc_rcv_cap(0);

    ++reserve_cap_count;

    return L4_EOK;
  }

  void free_epiface(L4::Epiface *) override
  {
    ++free_epiface_count;
  }

  void free_capability(L4::Cap<void> cap) override
  {
    L4Re::Util::Unique_cap<void> s(cap);
    ++free_cap_count;
  }

  void reset_counters()
  {
    reserve_epiface_count = 0;
    free_epiface_count = 0;
    reserve_cap_count = 0;
    free_cap_count = 0;
  }

  L4Re::Util::Object_registry *registry;
  int reserve_epiface_count;
  int free_epiface_count;
  int reserve_cap_count;
  int free_cap_count;
};


