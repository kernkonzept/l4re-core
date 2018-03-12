/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#include <l4/re/util/name_space_svr>
#include <l4/re/util/debug>

#include <l4/re/namespace>

#include <cassert>
#include <cstring>

namespace L4Re { namespace Util { namespace Names {

bool
Name::operator < (Name const &r) const
{
  unsigned long l = cxx::min(len(), r.len());
  int v = memcmp(start(), r.start(), l);
  return v < 0 || (v == 0 && len() < r.len());
}

void
Entry::set(Obj const &o)
{
  obj()->set(o, obj()->flags());
}

Entry *
Name_space::find_iter(Name const &pname) const
{
  Name name = pname;
  _dbg.printf("resolve '%.*s': ", name.len(), name.start());
  Name_space const *ns = this;
  while (ns)
    {
      cxx::String::Index sep = name.find("/");
      cxx::String part;
      if (!name.eof(sep))
	part = name.head(sep);
      else
	part = name;

      _dbg.cprintf(" '%.*s'", part.len(), part.start());
      Entry *o = ns->find(Name(part.start(), part.len()));

      if (!o)
	{
	  _dbg.cprintf(": resolution failed: '%.*s' remaining\n",
	              name.len(), name.start());
	  return 0;
	}

      ns = dynamic_cast<Name_space const *>(o->obj()->obj());
      if (ns)
	{
	  if (!name.eof(sep))
	    {
	      name = name.substr(sep + 1);
	      continue;
	    }
	}

      _dbg.cprintf(": found object: %p (%s)\n", o->obj()->obj(), o->obj()->obj()?typeid(*(o->obj()->obj())).name():"");

      return o;
    }

  return 0;
}


int
Name_space::op_query(L4Re::Namespace::Rights,
                     L4::Ipc::Array_in_buf<char, unsigned long> const &name,
                     L4::Ipc::Snd_fpage &snd_cap, L4::Ipc::Opt<L4::Opcode> &dummy,
                     L4::Ipc::Opt<L4::Ipc::Array_ref<char, unsigned long> > &out_name)
{
#if 1
  _dbg.printf("query: [%ld] '%.*s'\n", name.length, (int)name.length, name.data);
#endif

  char const *sep = (char const*)memchr(name.data, '/', name.length);
  unsigned long part;
  if (sep)
    part = sep - name.data;
  else
    part = name.length;

  Entry *n = find(Name(name.data, part));
  if (!n)
    return -L4_ENOENT;
  else if (!n->obj()->is_valid())
    return -L4_EAGAIN;
  else
    {
      if (n->obj()->cap().validate(L4_BASE_TASK_CAP).label() <= 0)
        {
          if (n->obj()->is_local())
            free_epiface(n->obj()->obj());
          else
            free_capability(n->obj()->cap());

          if (n->is_dynamic())
            {
              remove(n->name());
              free_dynamic_entry(n);
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
      if (n->obj()->is_rw())     flags |= L4_CAP_FPAGE_W;
      if (n->obj()->is_strong()) flags |= L4_CAP_FPAGE_S;

      snd_cap = L4::Ipc::Snd_fpage(n->obj()->cap(), flags);
      _dbg.printf(" result = %lx flgs=%x strg=%d\n",
                  result, flags, (int)n->obj()->is_strong());
      return result;
    }
}

int
Name_space::insert_entry(Name const &name, unsigned flags, Entry **e)
{
  Entry *n = find(name);
  if (n && n->obj()->is_valid())
    {
      if (!(flags & L4Re::Namespace::Overwrite)
	  && n->obj()->cap().validate(L4_BASE_TASK_CAP).label() > 0)
	return -L4_EEXIST;

      if (n->obj()->is_local())
        free_epiface(n->obj()->obj());
      else
	free_capability(n->obj()->cap());

      if (n->is_dynamic())
	{
	  remove(n->name());
	  free_dynamic_entry(n);
	  n = 0;
	}
      else
	{
	  if (!n->obj()->is_replacable())
	    return -L4_EEXIST;
          n->obj()->reset(Obj::F_rw);
	}
    }

  flags &= L4Re::Namespace::Cap_flags;
  if (!n)
    {
      if (!(n = alloc_dynamic_entry(name, flags)))
	return -L4_ENOMEM;
      else
	{
	  int err = insert(n);
	  if (err < 0)
	    {
	      free_dynamic_entry(n);
	      return err;
	    }
	}
    }

  *e = n;
  return 0;
}

int
Name_space::op_register_obj(L4Re::Namespace::Rights, unsigned flags,
                            L4::Ipc::Array_in_buf<char, unsigned long> const &name,
                            L4::Ipc::Snd_fpage &cap)
{
  if (name.length == 0 || memchr(name.data, '/', name.length))
      return -L4_EINVAL;

  L4::Cap<void> reg_cap(L4_INVALID_CAP);
  L4::Epiface *src_o = 0;

  // Did we receive something we have handed out ourselves? If yes,
  // register the object under the given name but do not allocate
  // anything more.
  if (cap.id_received() || cap.local_id_received())
    {
      if (int ret = get_epiface(cap.data(), cap.local_id_received(), &src_o))
        return ret;

      // Make sure rights are restricted to the mapped rights.
      flags &= (cap.data() & 0x3UL) | ~0x3UL;
    }
  else if (cap.cap_received())
    {
      if (int ret = copy_receive_cap(&reg_cap))
        return ret;
    }
  else if (!cap.is_valid())
    {
      reg_cap = L4::Cap<void>::Invalid;
    }
  else
    return -L4_EINVAL;

  // got a valid entry to register
  _dbg.printf("register: '%.*s' flags=%x\n", (int)name.length, name.data, flags);

  Name _name(name.data, name.length);

  Entry *n;
  if (int r = insert_entry(_name, flags, &n))
    {
      if (cap.cap_received())
        free_capability(reg_cap);
      if (src_o)
        free_epiface(src_o);

      return r;
    }

  if (src_o)
    n->set(Names::Obj(flags & L4Re::Namespace::Cap_flags, src_o));
  else if (reg_cap.is_valid())
    n->set(Names::Obj(flags & L4Re::Namespace::Cap_flags, reg_cap));

  return 0;
}

int
Name_space::op_unlink(L4Re::Namespace::Rights,
                      L4::Ipc::Array_in_buf<char, unsigned long> const &name)

{
#if 1
  _dbg.printf("unlink: [%ld] '%.*s'\n", name.length, (int)name.length, name.data);
#endif

  char const *sep = (char const*)memchr(name.data, '/', name.length);
  unsigned long part;
  if (sep)
    part = sep - name.data;
  else
    part = name.length;

  Entry *n = find(Name(name.data, part));
  if (!n || !n->obj()->is_valid())
    return -L4_ENOENT;

  if (n->obj()->is_local())
    free_epiface(n->obj()->obj());
  else
    free_capability(n->obj()->cap());

  if (n->is_dynamic())
    {
      remove(n->name());
      free_dynamic_entry(n);
    }
  else
    return -L4_EACCESS;

  return 0;
}




}}}
