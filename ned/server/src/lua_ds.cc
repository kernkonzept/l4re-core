/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author: Christian Pötzsch <christian.poetzsch@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/*
 * Lua methods of dataspace capabilities.
 *
 *   ds:size()                                  size of ds in bytes
 *   ds:copy_in(dst_offs, src, src_offs, size)  copy size bytes of the
 *                                              dataspace src at src_offs to
 *                                              dst_offs of ds
 *
 * E.g. a fresh copy of a boot module:
 * local src = L4.Env.rom:query("data", L4.Rights.ronly)();
 * local ds = L4.Env.user_factory:create(L4.Proto.Dataspace, src:size()):m("rw");
 * ds:copy_in(0, src, 0, src:size());
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <l4/re/dataspace>

#include "lua_cap.h"
#include "lua.h"

namespace Lua { namespace {

static int
__size(lua_State *l)
{
  Lua::Cap *n = check_cap(l, 1);
  L4Re::Dataspace::Stats stats;
  l4_ret_t r = n->cap<L4Re::Dataspace>()->info(&stats);
  if (r < 0)
    luaL_error(l, "runtime error %s (%d)", l4sys_errtostr(r), r);

  lua_pushinteger(l, stats.size);
  return 1;
}

static int
__copy_in(lua_State *l)
{
  Lua::Cap *n = check_cap(l, 1);
  lua_Integer dst_offs = luaL_checkinteger(l, 2);
  Lua::Cap *src = check_cap(l, 3);
  lua_Integer src_offs = luaL_checkinteger(l, 4);
  lua_Integer size = luaL_checkinteger(l, 5);

  if (dst_offs < 0 || src_offs < 0 || size < 0)
    luaL_error(l, "negative offset or size");

  auto src_cap = L4::Ipc::make_cap(src->cap<L4Re::Dataspace>().get(),
                                   src->rights());
  l4_ret_t r = n->cap<L4Re::Dataspace>()->copy_in(dst_offs, src_cap,
                                                  src_offs, size);
  if (r < 0)
    luaL_error(l, "runtime error %s (%d)", l4sys_errtostr(r), r);

  return 0;
}

struct Ds_model
{
  static void
  register_methods(lua_State *l)
  {
    static const luaL_Reg l4_cap_class[] =
    {
      { "size", __size },
      { "copy_in", __copy_in },
      { NULL, NULL }
    };
    luaL_setfuncs(l, l4_cap_class, 0);
    Cap::add_class_metatable(l);
  }
};

static Lua::Cap_type_lib<L4Re::Dataspace, Ds_model> __lib;

}}
