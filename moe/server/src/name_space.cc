/*
 * (c) 2008-2009 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/cxx/iostream>
#include "name_space.h"
#include "debug.h"
#include "globals.h"
#include "string.h"
#include "server_obj.h"

#include <l4/cxx/l4iostream>
#include <l4/cxx/minmax>
#include <l4/cxx/unique_ptr>

#include <cstring>
#include <cstdlib>
#include <cassert>

Moe::Name_space *root_name_space()
{
  static auto *_root = Moe::Moe_alloc::allocator()->make_obj<Moe::Name_space>();
  return _root;
}

namespace Moe {

static Dbg dbg(Dbg::Name_space, "ns");

Entry::~Entry()
{
  qalloc()->free(const_cast<char *>(_name.start()));

  if (_flags & F_allocated)
    object_pool.cap_alloc()->free(cap());

  // explicit destructor call required because of union
  if (is_local())
    _obj.~Weak_ref();
}


void
Entry::set(L4::Cap<void> cap)
{
  _cap = cap.cap();
  _flags = (_flags | F_cap) & ~F_local;
}

void
Entry::set(Moe::Server_object *o)
{
  new (&_obj) Weak_ref(o);

  _flags |= F_local | F_cap;
}

void
Entry::set_epiface(l4_umword_t data)
{
  assert(!is_valid());

  auto *ef = static_cast<Moe::Server_object*>(object_pool.find(data));

  if (!ef)
    throw L4::Runtime_error(-L4_EINVAL);

  set(ef);

  // make sure rights are restricted to the mapped rights
  _flags &= (data & 0x3UL) | ~0x3UL;
}

void
Entry::set_cap_copy(L4::Cap<L4::Kobject> cap)
{
  assert(!is_valid());

  auto nc = object_pool.cap_alloc()->alloc();
  nc.move(cap);

  set(nc);
  _flags |= F_allocated | F_cap;
}


Name_space::~Name_space()
{
  _tree.remove_all([](Entry *e) { delete e; });
}

Entry *
Name_space::check_existing(Name_buffer const &name, unsigned flags)
{
  Entry *n = find(Entry::Name(name.data, name.length));
  if (n)
    {
      if (!n->is_valid())
        return n;

      if (!n->is_dynamic())
        throw L4::Element_already_exists();

      if (!(flags & L4Re::Namespace::Overwrite)
          && n->cap().validate(L4_BASE_TASK_CAP).label() > 0)
        throw L4::Element_already_exists();

      return n;
    }

  return 0;
}

Entry *
Name_space::find_iter(Entry::Name const &pname) const
{
  Entry::Name name = pname;
  dbg.printf("resolve '%.*s': ", name.len(), name.start());
  Name_space const *ns = this;
  while (ns)
    {
      cxx::String::Index sep = name.find("/");
      cxx::String part;
      if (!name.eof(sep))
        part = name.head(sep);
      else
        part = name;

      dbg.cprintf(" '%.*s'", part.len(), part.start());
      Entry *o = ns->find(Entry::Name(part.start(), part.len()));

      if (!o || !o->is_local())
        {
          dbg.cprintf(": resolution failed: '%.*s' remaining\n",
                      name.len(), name.start());
          return 0;
        }

      ns = dynamic_cast<Name_space const *>(o->_obj.get());
      if (ns)
        {
          if (!name.eof(sep))
            {
              name = name.substr(sep + 1);
              continue;
            }
        }

      dbg.cprintf(": found object: %p (%s)\n", o->_obj.get(),
                  o->_obj ? typeid(*(o->_obj.get())).name() : "");

      return o;
    }

  return 0;
}


int
Name_space::op_register_obj(L4Re::Namespace::Rights, unsigned flags,
                            Name_buffer const &name, L4::Ipc::Snd_fpage &cap)
{
  if (name.length == 0 || memchr(name.data, '/', name.length))
    return -L4_EINVAL;

  if (cap.local_id_received())
    return -L4_EINVAL;


  // check if we are are going to overwrite
  Entry *existing = check_existing(name, flags);
  // make ourselves a new entry
  cxx::unique_ptr<Entry> n(create_entry(name, flags));

  if (cap.id_received())
    n->set_epiface(cap.data());
  else if (cap.cap_received())
    n->set_cap_copy(L4::Cap<L4::Kobject>(Rcv_cap << L4_CAP_SHIFT));
  else if (cap.is_valid())
    // received a valid cap we cannot handle
    return -L4_EINVAL;

  // insert, overwriting if necessary
  if (existing)
    {
      // clean up the overwritten entry
      cxx::unique_ptr<Entry> to_delete(existing);
      remove(existing->name());
    }

  bool r = insert(n.get());
  assert(r);
  n.release();

  return L4_EOK;
}

int
Name_space::op_unlink(L4Re::Namespace::Rights, Name_buffer const &name)
{
  auto *sep = (char const *)memchr(name.data, '/', name.length);
  unsigned long part;
  if (sep)
    part = sep - name.data;
  else
    part = name.length;

  Entry *n = find(Entry::Name(name.data, part));
  if (!n)
    return -L4_ENOENT;

  if (!n->is_dynamic())
    return -L4_EACCESS;

  remove(n->name());

  // get rid of the entry
  cxx::unique_ptr<Entry> to_delete(n);

  return L4_EOK;
}

int
Name_space::op_query(L4Re::Namespace::Rights,
                     Name_buffer const &name,
                     L4::Ipc::Snd_fpage &snd_cap,
                     L4::Ipc::Opt<L4::Opcode> &dummy,
                     L4::Ipc::Opt<L4::Ipc::Array_ref<char, unsigned long> > &out_name)
{
#if 0
  dbg.printf("query: [%ld] '%.*s'\n", name.length, (int)name.length, name.data);
#endif

  auto *sep = (char const *)memchr(name.data, '/', name.length);
  unsigned long part;
  if (sep)
    part = sep - name.data;
  else
    part = name.length;

  Entry *n = find(Entry::Name(name.data, part));
  if (!n)
    return -L4_ENOENT;
  if (!n->is_valid())
    return -L4_EAGAIN;

  if (n->cap().validate(L4_BASE_TASK_CAP).label() <= 0)
    {
      if (n->is_dynamic())
        {
          cxx::unique_ptr<Entry> old(n);
          remove(n->name());
        }
      return -L4_ENOENT;
    }

  // make picky clients happy
  dummy.set_valid();

  l4_umword_t result = 0;

  out_name.set_valid();
  if (part < name.length)
    {
      result |= L4Re::Namespace::Partly_resolved;
      memcpy(out_name->data, name.data + part + 1, name.length - part - 1);
      out_name->length = name.length - part - 1;
    }
  else
    out_name->length = 0;

  unsigned flags = L4_CAP_FPAGE_R;
  if (n->is_rw())     flags |= L4_CAP_FPAGE_W;
  if (n->is_strong()) flags |= L4_CAP_FPAGE_S;

  snd_cap = L4::Ipc::Snd_fpage(n->cap(), flags);
  dbg.printf(" result = %lx flgs=%x strg=%d\n",
              result, flags, (int)n->is_strong());
  return result;
}


void
Name_space::dump(bool rec, int indent) const
{
  Name_space const *n;
  //L4::cout << "MOE: Name space dump (" << obj_cap() << ")\n";
  for (Const_iterator i = begin(); i != end(); ++i)
    {
      for (int x = 0; x < indent; ++x)
        L4::cout << "  ";

      L4::cout << "  " << i->name()  << " -> " << i->cap();
      if (i->is_local())
        L4::cout << " o=" << (void *)i->obj();
      L4::cout << " f=" << i->_flags << '\n';
      if (rec && i->is_local()
          && (n = dynamic_cast<Name_space const *>(i->obj())))
        {
          n->dump(rec, indent + 1);
        }
    }
}

}
