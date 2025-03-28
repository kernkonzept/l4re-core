/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#include <l4/re/env>
#include <l4/sys/kip.h>
#include <l4/util/kip.h>
#include <l4/bid_config.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "lua.h"
#include "lua_cap.h"

namespace Lua {
namespace {

class Lib_info : public Lib
{
public:
  Lib_info() : Lib(P_env) {}

  static int kipstr(lua_State *l)
  {
    const char *s = l4_kip_version_string(l4re_kip());
    lua_pushstring(l, s);
    return 1;
  }

  static int archstr(lua_State *l)
  {
    lua_pushstring(l, CONFIG_BUILD_ARCH);
    return 1;
  }

  static int platformstr(lua_State *l)
  {
    lua_pushstring(l, l4re_kip()->platform_info.name);
    return 1;
  }

  static int mword_bits(lua_State *l)
  {
    lua_pushinteger(l, sizeof(l4_umword_t) * 8);
    return 1;
  }

  void init(lua_State *l)
  {
    lua_require_module(l, package);
    // L4.Info
    lua_newtable(l);

    // L4.Info.Kip
    lua_newtable(l);
    // L4.Info.Kip.str()
    lua_pushcfunction(l, kipstr);
    lua_setfield(l, -2, "str");
    lua_setfield(l, -2, "Kip");

    // L4.Info.arch()
    lua_pushcfunction(l, archstr);
    lua_setfield(l, -2, "arch");

    // L4.Info.platform()
    lua_pushcfunction(l, platformstr);
    lua_setfield(l, -2, "platform");

    // L4.Info.mword_size()
    lua_pushcfunction(l, mword_bits);
    lua_setfield(l, -2, "mword_bits");

    lua_setfield(l, -2, "Info");

    lua_pop(l, 2);
  }
};

static Lib_info __info_lib;

}}
