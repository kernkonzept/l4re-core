/*
 * Copyright (C) 2018 Kernkonzept GmbH.
 * Author: Steffen Liebergeld <steffen.liebergeld@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/*
 * This file implements support for the L4Re::Debug protocol.
 *
 * This is an example ned config that makes moe print its debug information:
 * local L4 = require("L4")
 *
 * L4.cast("L4Re::Debug_obj", L4.Env.user_factory):debug(0)
 */
#include <lua.h>
#include <lualib.h>
#include "lua.h"

#include <l4/re/debug>

#include "lua_cap.h"

namespace Lua { namespace {

static int
__debug(lua_State *l)
{
  Cap *_debug_obj = check_cap(l, 1);
  l4_umword_t n = luaL_checkinteger(l, 2);
  L4::Cap<L4Re::Debug_obj> debug_obj = _debug_obj->cap<L4Re::Debug_obj>().get();
  unsigned long ret = debug_obj->debug(n);
  lua_pushinteger(l, ret);
  return 1;
}

struct Debug_obj_model
{
  static void
  register_methods(lua_State *l)
  {
    static const luaL_Reg l4_cap_class[] =
      {
        { "debug", __debug },
        { NULL, NULL }
      };
    luaL_setfuncs(l, l4_cap_class, 0);
    Cap::add_class_metatable(l);
  }
};

static Lua::Cap_type_lib<L4Re::Debug_obj, Debug_obj_model> __lib;

}}
