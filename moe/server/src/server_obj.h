/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/cxx/hlist>
#include <l4/cxx/weak_ref>
#include <l4/sys/cxx/ipc_epiface>


namespace Moe
{

class Server_object
: public L4::Epiface,
  public cxx::H_list_item_t<Server_object>
{
public:
  /**
   * List of dynamic server objects.
   *
   * The list is ordered by factory ownership. All objects that
   * have been allocated with a given factory follow directly
   * after the factory object in the list. This property also holds
   * recursively.
   */
  typedef cxx::H_list_t<Server_object> Obj_list;

  virtual ~Server_object();

  void add_weak_ref(cxx::Weak_ref_base *obj) const;
  void remove_weak_ref(cxx::Weak_ref_base *obj) const;

private:
  mutable cxx::Weak_ref_base::List _weak_ptrs;
  mutable L4::Cap<void> _weak_cap;
};

/**
 * Handler class for stale requests from servers that have been
 * deregistered.
 */
struct Null_handler : L4::Epiface_t<Null_handler, L4::Kobject>
{};

} // namespace
