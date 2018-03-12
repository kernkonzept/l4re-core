/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/cxx/avl_tree>
#include <l4/cxx/std_ops>
#include <l4/cxx/unique_ptr>
#include <l4/sys/cxx/ipc_epiface>

#include <l4/sys/capability>
#include <l4/re/util/name_space_svr>

#include "server_obj.h"
#include "quota.h"
#include "obj_reg.h"
#include <l4/cxx/string>

#include <cstring>
#include <cstdlib>


namespace Moe {

class Name_space;

class Entry
: public cxx::Avl_tree_node,
  public Q_object
{
public:
  friend class Name_space;
  typedef cxx::String Name;

  enum Flags
  {
    F_rw        = L4Re::Namespace::Rw,
    F_strong    = L4Re::Namespace::Strong,

    F_trusted   = L4Re::Namespace::Trusted,

    F_rights_mask = F_rw | F_strong | F_trusted,

    F_cap        = 0x100,
    F_local      = 0x200,
    F_allocated  = 0x400,
    F_static     = 0x800,
    F_base_mask  = 0xf00,
  };

private:
  typedef cxx::Weak_ref<Moe::Server_object> Weak_ref;
  Name _name;
  unsigned _flags;
  union
  {
    l4_cap_idx_t _cap;
    Weak_ref _obj;
  };

  void set(L4::Cap<void> cap);
  void set(Moe::Server_object *o);

public:
  Entry(char const *name, unsigned flags)
  : Entry(name, strlen(name), flags)
  {}

  Entry(char const *name, unsigned long len, unsigned flags)
  : _flags(flags)
  {
    auto namecpy = qalloc()->alloc<char>(len + 1);
    memcpy(namecpy, name, len);
    namecpy[len] = 0;
    _name = Name(namecpy, len);
  }

  ~Entry();

  Name const &name() const
  { return _name; }

  bool is_dynamic() const
  { return !(_flags & F_static); }

  bool is_valid() const
  { return (_flags & F_cap) && !(is_local() && !_obj); }

  bool is_local() const
  { return _flags & F_local; }

  bool is_rw() const
  { return (_flags & F_rw) == F_rw; }

  bool is_strong() const
  { return _flags & F_strong; }

  unsigned flags() const
  { return _flags; }

  L4::Cap<void> cap() const
  {
    if (!is_local())
      return L4::Cap<void>(_cap);
    if (!_obj)
      return L4::Cap<void>::Invalid;
    return _obj.get()->obj_cap();
  }

  L4::Epiface *obj() const
  { return _obj.get(); }

  void set_epiface(l4_umword_t data);
  void set_cap_copy(L4::Cap<L4::Kobject> cap);
};

struct Entry_get_key
{
  typedef Entry::Name Key_type;
  static Key_type const &key_of(Entry const *e)
  { return e->name(); }
};

struct Entry_key_compare
{
  bool operator () (Entry::Name const &l, Entry::Name const &r) const
    {
      unsigned long len = cxx::min(l.len(), r.len());
      int v = memcmp(l.start(), r.start(), len);
      return v < 0 || (v == 0 && l.len() < r.len());
  }
};


class Name_space :
  public L4::Epiface_t<Name_space, L4Re::Namespace, Moe::Server_object>,
  public Q_object
{
  friend class Entry;
  typedef cxx::Avl_tree<Entry, Entry_get_key, Entry_key_compare> Tree;
  typedef L4::Ipc::Array_in_buf<char, unsigned long> Name_buffer;
  Tree _tree;

  Entry *find(Entry::Name const &name) const
  { return _tree.find_node(name); }

  Entry *remove(Entry::Name const &name)
  { return _tree.remove(name); }

  bool insert(Entry *e)
  { return _tree.insert(e).second; }

  Entry *check_existing(Name_buffer const &name, unsigned flags);

  Entry *create_entry(Name_buffer const &name, unsigned flags)
  {
    return qalloc()->make_obj<Entry>(name.data, name.length,
                                     flags & Entry::F_rights_mask);
  }
public:
  typedef Tree::Const_iterator Const_iterator;

  virtual ~Name_space();

  Const_iterator begin() const { return _tree.begin(); }
  Const_iterator end() const { return _tree.end(); }

  // server interface ------------------------------------------
  int op_query(L4Re::Namespace::Rights,
               Name_buffer const &name,
               L4::Ipc::Snd_fpage &snd_cap, L4::Ipc::Opt<L4::Opcode> &,
               L4::Ipc::Opt<L4::Ipc::Array_ref<char, unsigned long> > &out_name);

  int op_register_obj(L4Re::Namespace::Rights, unsigned flags,
                      Name_buffer const &name, L4::Ipc::Snd_fpage &cap);

  int op_unlink(L4Re::Namespace::Rights r, Name_buffer const &name);

  // internally used to register bootfs files, name spaces...
  template <typename T>
  int register_obj(Entry::Name const &name, unsigned long flags, T cap)
  {
    cxx::unique_ptr<Entry> n(qalloc()->make_obj<Entry>(name.start(), name.len(),
                                                       flags | Entry::F_static));
    n->set(cap);
    if (!insert(n.get()))
      return -L4_EEXIST;

    n.release();
    return 0;
  }

  Entry *find_iter(Entry::Name const &name) const;

  void dump(bool rec = false, int indent = 0) const;
};

}

Moe::Name_space *root_name_space();

