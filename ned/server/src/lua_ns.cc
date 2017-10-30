/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "lua.h"

#include <l4/re/namespace>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/env>

#include "lua_cap.h"

#include <cstring>
#include <cstdio>

namespace Lua { namespace {

static int
__lookup(lua_State *l)
{
  size_t len;
  char const *s = lua_tolstring(l, lua_upvalueindex(2), &len);
  Cap *_ns = Lua::check_cap(l, lua_upvalueindex(1));
  L4_cap_fpage_rights rights
    = (L4_cap_fpage_rights)lua_tointeger(l, lua_upvalueindex(3));

  L4::Cap<L4Re::Namespace> ns(_ns->cap<L4Re::Namespace>().get());

  auto obj = L4Re::Util::make_unique_cap<void>();
  if (!obj.is_valid())
    luaL_error(l, "out of caps");

  lua_gc(l, LUA_GCCOLLECT, 0);

  int r = ns->query(s, len, obj.get(), L4Re::Namespace::To_default);
#if 0
  if (r == -L4_ENOENT)
    return Cap::index(l);
#endif
  if (r == -L4_ENOENT)
    {
      lua_pushnil(l);
      return 1;
    }

  if (r < 0)
    luaL_error(l, "runtime error %s (%d)\n", l4sys_errtostr(r), r);


  Lua::Cap *no = Lua::push_void_cap(l);
  no->set(obj.release());
  no->set_rights(rights);

  return 1;

}

static int
__query(lua_State *l)
{
  //size_t len;
  //char const *s = luaL_checklstring(l, 2, &len);
  int argc = lua_gettop(l);
  if (argc < 2)
    luaL_error(l, "expected at least two arguments in name-space query");

  lua_pushvalue(l, 1);
  lua_pushvalue(l, 2);
  if (argc > 2)
    lua_pushvalue(l, 3);
  else
    lua_pushinteger(l, L4_FPAGE_RO);

  lua_pushcclosure(l, __lookup, 3);
  return 1;
}

static int
__register(lua_State *l)
{
  Cap *ns = check_cap(l, 1);
  char const *key = luaL_checkstring(l, 2);

  Cap *n = 0;
  lua_Integer type = lua_type(l, 3);
  lua_gc(l, LUA_GCCOLLECT, 0);
  switch (type)
    {
    case LUA_TNIL:
	{
	  int r = ns->cap<L4Re::Namespace>()->unlink(key);
	  if (r >= 0 || r == -L4_ENOENT)
	    return 1;

	  luaL_error(l, "runtime error %s (%d)", l4sys_errtostr(r), r);
	}
    case LUA_TSTRING:
      break;

    case LUA_TUSERDATA:
      n = Lua::check_cap(l, 3);
      break;
    default:
      luaL_error(l, "unexpected value to register in namespace (%s)",
                 lua_typename(l, 3));
    }

  if (0)
    printf("register %s=%lx in %lx\n",
           key, n ? n->cap<void>().cap() : ~0UL, ns->cap<void>().cap());

  int r;
  L4::Cap<void> cap = n ? n->cap<void>().get() : L4::Cap<void>::Invalid;
  unsigned rights = n ? n->rights() : L4_CAP_FPAGE_RO;
  r = ns->cap<L4Re::Namespace>()
        ->register_obj(key, L4::Ipc::make_cap(cap, rights),
                       L4Re::Namespace::Overwrite | rights);
  if (r < 0)
    luaL_error(l, "runtime error %s (%d)", l4sys_errtostr(r), r);

  return 1;
}

struct Ns_model
{
  static void
  register_methods(lua_State *l)
  {
  static const luaL_Reg l4_cap_class[] =
    {
      { "q", __query },
      { "query", __query },
      { "__register", __register },
      { NULL, NULL }
    };
  luaL_setfuncs(l, l4_cap_class, 0);
  Cap::add_class_metatable(l);
  }
};


static Lua::Cap_type_lib<L4Re::Namespace, Ns_model> __lib;

}}

