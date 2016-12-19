/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include <l4/re/env>
#include <l4/re/namespace>
#include <l4/sys/factory>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "lua.h"
#include "lua_cap.h"

namespace Lua {
namespace {

class Lib_env : public Lib
{
public:
  Lib_env() : Lib(P_env) {}

  void init(lua_State *l)
  {
#if 0
    printf("initializing lua L4Re::Env\n");
#endif
    Lua::lua_require_module(l, "L4");
    lua_newtable(l);
    lua_pushvalue(l, -1);
    lua_setfield(l, -3, "Env");

    L4Re::Env const *e = L4Re::Env::env();

    register_cap(l, "parent", e->parent());
    register_cap(l, "mem_alloc", e->mem_alloc(), L4::Factory::Protocol);
    register_cap(l, "rm", e->rm());
    register_cap(l, "log", e->log());
    register_cap(l, "factory", e->factory(), L4::Factory::Protocol);
    register_cap(l, "scheduler", e->scheduler());

    for (L4Re::Env::Cap_entry const *i = e->initial_caps();
         i && i->flags != ~0UL; ++i)
      {
	register_cap(l, i->name, L4::Cap<L4::Kobject>(i->cap));
      }

    lua_pop(l, 2);
  }
};

static Lib_env __env_lib;

}}
