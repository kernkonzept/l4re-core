/*
 * (c) 2010 Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "lua.h"
#include "lua_cap.h"
#include "debug.h"


#include <l4/re/util/cap_alloc>
#include <l4/sys/types.h>
#include <l4/sys/meta>

namespace Lua {

void
get_cap_cast_table(lua_State *l)
{
  lua_getglobal(l, package);
  lua_getfield(l, LUA_REGISTRYINDEX, "L4::Ned::TYPES");
  lua_remove(l, -2);
}

Cap::~Cap()
{
}

void
Cap::create_class(lua_State *l, Register_methods *rm, char const *type)
{
  lua_newtable(l);
  rm(l);
  lua_pushstring(l, type);
  lua_setfield(l, -2, "_CLASS_NAME");
}

void
Cap::get_class(lua_State *l)
{
  get_cap_cast_table(l);
  lua_getfield(l, -1, "void");
  lua_remove(l, -2);
}

static int
is_valid(lua_State *l)
{
  Cap *n = check_cap(l, 1);
  lua_pushboolean(l, n && n->cap<void>().is_valid());
  return 1;
}

static int
gc_cap(lua_State *l)
{
  Lua::Cap *n = check_cap(l, 1);
  n->~Cap();
  return 1;
}


static int
tostring(lua_State *l)
{
  Lua::Cap *n = check_cap(l, 1);
  lua_getuservalue(l, 1);
  lua_rawgeti(l, -1, 0);
  char const *type = "void";
  if (!lua_isnil(l, -1))
    {
      lua_getfield(l, -1, "_CLASS_NAME");
      type = lua_tostring(l, -1);
    }

  if (n->cap<void>().is_valid())
    lua_pushfstring(l, "L4::Cap<%s>[%p] r=%p f=%p", type,
                    (void*)(n->cap<void>().cap()),
                    (void*)(l4_addr_t)n->all_rights(), (void*)(l4_addr_t)n->flags());
  else
    lua_pushfstring(l, "L4::Cap<%s>::Invalid", type);
  return 1;
}

static Cap *
__set_rights(lua_State *l, unsigned r)
{
  Lua::Cap *n = check_cap(l, 1);
  if (n->all_rights() == r)
    lua_pushvalue(l, 1);
  else
    {
      Lua::Cap *t = n->clone(l);
      t->set_rights(r);
      lua_getuservalue(l, 1);
      lua_setuservalue(l, -2);
      return t;
    }

  return n;
}

static int
__set_mode_call(lua_State *l)
{
  Cap *c;
  if (lua_gettop(l) >= 2 && !lua_isnil(l, 2))
    {
      unsigned rights = L4_CAP_FPAGE_R;
      if (lua_isnumber(l, 2))
	rights = lua_tointeger(l, 2);
      else
	{
	  for (char const *r = lua_tostring(l, 2); *r; ++r)
	    {
	      switch (*r)
		{
		case 'r':
		case 'R': rights |= L4_CAP_FPAGE_R; break;
		case 'w':
		case 'W': rights |= L4_CAP_FPAGE_W; break;
		case 's':
		case 'S': rights |= L4_CAP_FPAGE_S; break;
		case 'd':
		case 'D': rights |= L4_CAP_FPAGE_D; break;
		case 'n':
		case 'N': rights |= 0x10; break;
		case 'c':
		case 'C': rights |= 0x20; break;
		}
	    }
	}
      __set_rights(l, rights);
      return 1;
    }
  else
    c = check_cap(l, 1);

  lua_pushinteger(l, c->rights());

  return 1;
}


static int
__full(lua_State *l)
{
  __set_rights(l, 0xef);
  return 1;
}


bool
Cap::find_dynamic_type(lua_State *l) const
{
  using L4Re::Util::Ref_cap;
  Dbg dbg(Dbg::Warn, "lua");
  Ref_cap<L4::Meta>::Cap _meta = L4::cap_cast<L4::Meta>(_c);
  long proto = 0;
  char name_buf[256];
  L4::Ipc::String<char> name(sizeof(name_buf), name_buf);
  int err = l4_error(_meta->interface(0, &proto, &name));
  if (err < 0)
    {
      dbg.printf("Warning: Capability %lx does not support the meta protocol: %d\n",
                 _c.cap(), err);
      return false;
    }

  get_method_table(l, name.data);
  if (lua_isnil(l, -1))
    { // no lua representation of type found
      lua_pop(l, -1);
      return false;
    }

  return true;
}

int
Cap::index(lua_State *l) const
{
  get_method_table(l, "void"); // push table
  lua_pushvalue(l, 2); // push key
  lua_gettable(l, -2);

  if (!lua_isnil(l, -1))
    return 1;

  lua_pop(l, 2); // pop nil result, and method table

  lua_getuservalue(l, 1);
  lua_rawgeti(l, -1, 0);
  if (lua_isnil(l, -1))
    {
      lua_pop(l, 1);
      if (!find_dynamic_type(l))
	return 0;

      lua_pushvalue(l, -1); // keep table after set
      lua_rawseti(l, -3, 0);
    }

  lua_pushvalue(l, 2); // push key
  lua_gettable(l, -2);
  return 1;
}

int
Cap::newindex(lua_State *) const
{ return 0; }



static int __index(lua_State *l)
{
  Lua::Cap *n = Lua::check_cap(l, 1);
  return n->index(l);
}

static
void init_cap_metatable(lua_State *l)
{
  Lua::register_method(l, "__gc", gc_cap);
  Lua::register_method(l, "__tostring", tostring);
  Lua::register_method(l, "__index", __index);
}

static
void push_cap_metatable(lua_State *l)
{
  if (luaL_newmetatable(l, CAP_TYPE))
    init_cap_metatable(l);
}

void set_cap_metatable(lua_State *l)
{
  push_cap_metatable(l);
  lua_setmetatable(l, -2);
}

int
Cap::get_method_table(lua_State *l, char const *typ) const
{
  get_cap_cast_table(l);
  lua_getfield(l, -1, typ /*type()*/);
  lua_remove(l, -2);
  return 1;
}

#if 0
  if (luaL_newmetatable(l, type()))
    {
      luaL_register(l, NULL, l4_cap_class);
      return 1;
    }
  return 0;
}
#endif

Cap *
push_void_cap(lua_State *l)
{
  Cap *c = new (lua_newuserdata(l, sizeof(Cap))) Cap();
  set_cap_metatable(l);
  lua_newtable(l);
  lua_setuservalue(l, -2);
  return c;
}


Cap *
push_new_cap(lua_State *l, bool void_cap)
{
  int const frame = lua_gettop(l);
  get_cap_cast_table(l);
  lua_pushvalue(l, frame);
  //long proto = lua_tointeger(l, frame);
  lua_remove(l, frame);
  lua_gettable(l, frame);
  lua_remove(l, frame); // discard cast table

  // and the type table or nil at index 'frame'

  Cap *nc = 0;
  bool found_type;
  if ((found_type = !lua_isnil(l, frame)) || void_cap)
    nc = push_void_cap(l); // cap at frame + 1

  if (found_type)
    {
      lua_getuservalue(l, frame + 1);
      lua_pushvalue(l, frame);
      lua_rawseti(l, -2, 0);
      lua_pop(l, 1); // pop env table
    }

  lua_remove(l, frame); // cap or nothing at frame
  return nc;
}

void
register_cap(lua_State *l, char const *name, L4::Cap<void> i, long proto)
{
  lua_pushinteger(l, proto);
  Cap *c = push_new_cap(l, true);
  c->set(Cap::C<void>::Cap(i));
  c->set_rights(L4_CAP_FPAGE_RWS);
  lua_setfield(l, -2, name);
}

static int
__cast(lua_State *l)
{
  if (lua_gettop(l) > 2)
    luaL_error(l, "too many arguments to L4.cast (expected 2: type and cap)");

  if (lua_isfunction(l, 2))
    lua_call(l, 0, 1);

  Cap *c = check_cap(l, 2);
  lua_pushvalue(l, 1);
  Cap *nc = push_new_cap(l, false);
  if (nc)
    {
      nc->assign(c);
      return 1;
    }

  lua_pushnil(l);
  return 1;
}


void
init_lua_cap(lua_State *l)
{
  static luaL_Reg _cast_f[] =
  {
    { "__cast", __cast },
    { NULL, NULL }
  };

  Lua::lua_require_module(l, package);
  luaL_setfuncs(l, _cast_f, 0);
  lua_newtable(l);
  lua_pushvalue(l, -1);
  lua_setfield(l, -3, CAST_TABLE);
  lua_setfield(l, LUA_REGISTRYINDEX, "L4::Ned::TYPES");

  get_cap_cast_table(l);
  Cap::create_class(l, Cap::register_methods, "void");
  lua_setfield(l, -2, "void");
  lua_pop(l, 2);
}

void
Cap::register_methods(lua_State *l)
{
  static const luaL_Reg l4_cap_class[] =
  {
    { "is_valid", is_valid },
    { "svr",    __full },
    { "rights", __set_mode_call },
    { "mode",   __set_mode_call },
    { "m",      __set_mode_call },
    { NULL, NULL },
  };
  luaL_setfuncs(l, l4_cap_class, 0);
}

}
