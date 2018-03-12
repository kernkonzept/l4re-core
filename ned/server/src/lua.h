/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
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

int lua(int argc, char const* const* argv);

namespace Lua {

static char const *const package = "L4";
static luaL_Reg const empty_reg[]  = {{NULL, NULL}};

void lua_require_module(lua_State *l, char const *name);

inline void
register_method(lua_State *l, char const *name, lua_CFunction f)
{
  lua_pushcfunction(l, f);
  lua_setfield(l, -2, name);
}

class Lib
{
public:
  enum Prio
  {
    P_cap_type,
    P_env
  };

  explicit Lib(Prio);
  virtual ~Lib() {}

  virtual void init(lua_State *) = 0;
  Prio prio() const { return _prio; }

  static void run_init(lua_State *);

private:
  Prio _prio;
  Lib *_next;
};

}

