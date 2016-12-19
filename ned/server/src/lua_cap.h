/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <l4/re/util/cap_alloc>

#if 0
#include <cstdio>
#include <typeinfo>
#endif

#include "lua.h"

namespace Lua {

static char const *const CAP_TYPE = "L4::Cap";
static char const *const CAST_TABLE = "_CAP_TYPES";
void set_cap_metatable(lua_State *l);

#define L4_LUA_DECLARE_KOBJECT(_cap_type)                          \
  private:                                                         \
    void operator = (_cap_type const &);                           \
  public:                                                          \
    virtual _cap_type *clone(lua_State *l) const                   \
    {                                                              \
      _cap_type *c = new (lua_newuserdata(l, sizeof(_cap_type)))   \
        _cap_type(*this);                                          \
      set_cap_metatable(l);                                        \
      return c;                                                    \
    }                                                              \
  private:


class Cap
{
  L4_LUA_DECLARE_KOBJECT(Cap)

public:
  template< typename T >
  struct C : public L4Re::Util::Ref_cap<T> {};

  template< typename T >
  typename C<T>::Cap cap() const
  { return L4::cap_cast<T>(_c); }

  l4_fpage_t fpage() const
  { return _c.fpage(_rights & L4_CAP_FPAGE_RWSD); }

  unsigned long ext_rights() const
  { return _rights & 0xf0; }

  unsigned flags() const { return _flags; }

  unsigned all_rights() const
  { return _rights; }

  L4_cap_fpage_rights rights() const
  { return L4_cap_fpage_rights(_rights & 0xf); }

  Cap(C<void>::Cap c = C<void>::Cap(), unsigned flags = 0)
  : _c(c), _rights(L4_FPAGE_RO), _flags(flags) {}

  template< typename T >
  void set(typename C<T>::Cap c)
  { _c = c; }

  void set(C<void>::Cap c)
  { _c = c; }

  void trim_rights(L4_cap_fpage_rights keep)
  { _rights = _rights & (keep | (~0U << 4)); }

  void set_flags(unsigned flags)
  { _flags |= flags; }

  void set_rights(unsigned char r)
  { _rights = r | L4_FPAGE_RO; }

  virtual ~Cap();

  int index(lua_State *l) const;
  int newindex(lua_State *l) const;
  int get_method_table(lua_State *l, char const *typ) const;

  void *operator new (size_t, void *p) throw() { return p; }
  void operator delete (void *) {}

  void assign(Cap *o)
  {
    if (o == this)
      return;

    _c = o->_c;
    _rights = o->_rights;
  }

public:
  typedef void Register_methods(lua_State *l);
  static void register_methods(lua_State *l);
  static void create_class(lua_State *l, Register_methods *rm,
                           char const *type);
  static void get_class(lua_State *l);


  static void add_class_metatable(lua_State *l)
  {
    lua_newtable(l);
    get_class(l);
    lua_setfield(l, -2, "__index");
    lua_setmetatable(l, -2);
  }

  bool find_dynamic_type(lua_State *) const;

private:
  C<void>::Cap _c;
  unsigned _rights : 16;
  unsigned _flags  : 16;
};


Cap *
push_new_cap(lua_State *l, bool void_cap = false);

Cap *
push_void_cap(lua_State *l);

void
register_cap(lua_State *l, char const *name, L4::Cap<void> i, long proto = 0);

inline
Cap *
check_cap(lua_State *l, int idx)
{ return (Cap*)luaL_checkudata(l, idx, CAP_TYPE); }

void get_cap_cast_table(lua_State *l);

template< typename C, typename Register_func >
void
register_cap_type(lua_State *l, Register_func r)
{
  get_cap_cast_table(l);
  L4::Type_info const *m = L4::kobject_typeid<C>();
  char const *class_name = m->name() ? m->name() : "<unk-class>";
  Cap::create_class(l, r, class_name);

  if (0)
    printf("register new cap type %s: '%s'\n", typeid(C).name(), class_name);
  long proto = m->proto();
  if (proto)
    {
      lua_pushinteger(l, proto);
      lua_pushvalue(l, -2);
      lua_settable(l, -4);
    }

  lua_setfield(l, -2, class_name);
  lua_pop(l, 1);
}

template< typename KO, typename Lua_model >
class Cap_type_lib : public Lib
{
public:
  Cap_type_lib() : Lib(P_cap_type) {}
  void init(lua_State *l)
  { register_cap_type<KO>(l, Lua_model::register_methods); }
};

void init_lua_cap(lua_State *l);

}


