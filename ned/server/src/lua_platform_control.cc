/*
 * (c) 2015 Alexander Warg <alexander.warg@kernkonzept.com>
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <l4/re/env>
#include <l4/sys/platform_control>

#include "lua_cap.h"
#include "lua.h"

#include <cstring>
#include <cstdio>

namespace Lua { namespace {

static int
__system_suspend(lua_State *l)
{
  Lua::Cap *n = check_cap(l, 1);
  l4_umword_t extras = luaL_checkinteger(l, 2);
  auto pfc = n->cap<L4::Platform_control>().get();
  l4_msgtag_t t = pfc->system_suspend(extras);
  int r = l4_error(t);

  if (r < 0)
    luaL_error(l, "runtime error %s (%d)", l4sys_errtostr(r), r);

  return 0;
}

static int
__system_shutdown(lua_State *l)
{
  Lua::Cap *n = check_cap(l, 1);
  l4_umword_t reboot = luaL_checkinteger(l, 2);
  auto pfc = n->cap<L4::Platform_control>().get();
  l4_msgtag_t t = pfc->system_shutdown(reboot);
  int r = l4_error(t);

  if (r < 0)
    luaL_error(l, "runtime error %s (%d)", l4sys_errtostr(r), r);

  return 0;
}

struct Pfc_model
{
  static void
  register_methods(lua_State *l)
  {
    static const luaL_Reg l4_cap_class[] =
    {
      { "system_suspend", __system_suspend },
      { "system_shutdown", __system_shutdown },
      { NULL, NULL }
    };
    luaL_setfuncs(l, l4_cap_class, 0);
    Cap::add_class_metatable(l);
  }
};

static Lua::Cap_type_lib<L4::Platform_control, Pfc_model> __lib;

}}
