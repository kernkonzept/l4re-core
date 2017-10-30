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

#include <l4/re/util/cap_alloc>
#include <l4/re/util/unique_cap>
#include <l4/re/env>
#include <l4/sys/factory>

#include "lua_cap.h"
#include "lua.h"

#include <cstring>
#include <cstdio>


namespace Lua { namespace {

static int
__alloc(lua_State *l)
{
  int argc = lua_gettop(l);
  Lua::Cap *n = check_cap(l, 1);
  int objt = luaL_checkinteger(l, 2);

  auto obj = L4Re::Util::make_unique_cap<L4::Kobject>();

  if (!obj.is_valid())
    luaL_error(l, "out of caps");

  L4::Cap<L4::Factory> f(n->cap<L4::Factory>().get());
  L4::Ipc::Varg args[argc-1];
  for (int i = 3; i <= argc; ++i)
    {
      if (lua_isnumber(l, i))
        args[i-3] = L4::Ipc::Varg((l4_mword_t)luaL_checkinteger(l, i));
      else if (lua_isstring(l, i))
        args[i-3] = L4::Ipc::Varg(luaL_checkstring(l, i));
      else if (lua_isnil(l, i))
        args[i-3] = L4::Ipc::Varg::nil();
    }
  args[argc-2] = L4::Ipc::Varg::nil();

  l4_msgtag_t t = L4::Factory::create_t::call(f, obj.get(), objt, args);
  int r = l4_error(t);

  if (r < 0)
    luaL_error(l, "runtime error %s (%d)", l4sys_errtostr(r), r);

  lua_pushinteger(l, objt);
  Cap *nc = Lua::push_new_cap(l, true);
  nc->set(obj.get());

  obj.release();
  return 1;
}


struct Factory_model
{
  static void
  register_methods(lua_State *l)
  {
    static const luaL_Reg l4_cap_class[] =
    {
      { "create", __alloc },
      { NULL, NULL }
    };
    luaL_setfuncs(l, l4_cap_class, 0);
    Cap::add_class_metatable(l);
  }
};

static Lua::Cap_type_lib<L4::Factory, Factory_model> __lib;

}}
