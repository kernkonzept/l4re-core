/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */

#include <l4/sys/err.h>
#include <l4/re/error_helper>
#include <l4/ned/cmd_control>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <getopt.h>

#include "lua_cap.h"
#include "lua.h"
#include "server.h"

static Lua::Lib *_lua_init;

extern char const _binary_ned_lua_start[];
extern char const _binary_ned_lua_end[];

static int _create_table(lua_State *l)
{ lua_newtable(l); return 1; }

void Lua::lua_require_module(lua_State *l, char const *name)
{
  luaL_requiref(l, name, _create_table, 0);
}

Lua::Lib::Lib(Prio prio) : _prio(prio), _next(0)
{
  Lib **f = &_lua_init;
  while (*f && (*f)->prio() < prio)
    f = &(*f)->_next;

  _next = *f;
  *f = this;
}

void
Lua::Lib::run_init(lua_State *l)
{
  for (Lib *c = _lua_init; c; c = c->_next)
    c->init(l);
}


static const luaL_Reg libs[] =
{
  { "_G", luaopen_base },
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_TABLIBNAME, luaopen_table},
  { LUA_IOLIBNAME, luaopen_io },
  { LUA_STRLIBNAME, luaopen_string },
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_DBLIBNAME, luaopen_debug},
  { NULL, NULL }
};

static char const *const options = "+e:";
static struct option const loptions[] =
  {
    { "execute", 1, NULL, 'e' },
    { 0, 0, 0, 0 }
  };

static int
execute_lua_buf(lua_State *l, char const *buf, size_t sz, char const *name)
{
  if (luaL_loadbuffer(l, buf, sz, name))
    {
      fprintf(stderr, "lua error: %s.\n", lua_tostring(l, -1));
      lua_pop(l, lua_gettop(l));
      return 1;
    }

  if (lua_pcall(l, 0, 1, 0))
    {
      fprintf(stderr, "lua error: %s.\n", lua_tostring(l, -1));
      lua_pop(l, lua_gettop(l));
      return 1;
    }

  lua_pop(l, lua_gettop(l));

  return 0;
}

class Command_dispatcher
: public L4::Epiface_t<Command_dispatcher, L4Re::Ned::Cmd_control>
{
public:
  Command_dispatcher(lua_State *l) : _lua(l) {}

  long op_execute(L4Re::Ned::Cmd_control::Rights,
                  L4::Ipc::String_in_buf<> cmd,
                  L4::Ipc::Array_ref<char> &res)
  {
    if (luaL_loadbuffer(_lua, cmd.data, cmd.length - 1, "argument"))
      {
        fprintf(stderr, "lua couldn't parse '%.*s': %s.\n",
                static_cast<int>(cmd.length) - 1,
                cmd.data, lua_tostring(_lua, -1));
        lua_pop(_lua, 1);

        return -L4_EINVAL;
      }

    if (lua_pcall(_lua, 0, 1, 0))
      {
        // errors during Factory::create() are returned as a table with
        // { "msg" = message, "code" = return value }
        // extract error message from table and push it on the stack
        if (lua_istable(_lua, -1))
          lua_getfield(_lua, -1, "msg");

        fprintf(stderr, "lua couldn't execute '%.*s': %s.\n",
                static_cast<int>(cmd.length) - 1,
                cmd.data, lua_tostring(_lua, -1));
        lua_pop(_lua, 1);

        return -L4_EIO;
      }

    size_t len;
    char const *res_txt = luaL_tolstring(_lua, -1, &len);
    if (len > res.length)
      len = res.length;

    memcpy(res.data, res_txt,  len);
    res.length = len;

    // drop the return value
    lua_pop(_lua, lua_gettop(_lua));

    return L4_EOK;
  }

private:
  lua_State *_lua;
};

namespace Lua { namespace {

static int __server_loop(lua_State *l)
{
  Lua::Cap *n = check_cap(l, 1);

  Command_dispatcher cmd_dispatch(l);

  L4Re::chkcap(Ned::server.registry()->register_obj(&cmd_dispatch,
                                                    n->cap<L4::Ipc_gate>().get()),
               "Register command dispatcher endpoint.");

  Ned::server_loop();

  Ned::server.registry()->unregister_obj(&cmd_dispatch);

  return 0;
}

class Lib_server : public Lib
{
public:
  Lib_server() : Lib(P_env) {}

  void init(lua_State *l)
  {
    static const luaL_Reg _ops[] =
    {
      { "server_loop", __server_loop },
      { NULL, NULL }
    };
    Lua::lua_require_module(l, "L4");
    luaL_setfuncs(l, _ops, 0);
  }
};
static Lib_server __libserver;

}}

int lua(int argc, char const *const *argv)
{
  printf("Ned says: Hi World!\n");

  lua_State *L;
  L = luaL_newstate();

  if (!L)
    return 1;

  for (int i = 0; libs[i].func; ++i)
    {
      luaL_requiref(L, libs[i].name, libs[i].func, 1);
      lua_pop(L, 1);
    }

  Lua::init_lua_cap(L);
  Lua::Lib::run_init(L);

  if (execute_lua_buf(L, _binary_ned_lua_start,
                      _binary_ned_lua_end - _binary_ned_lua_start,
                      "@ned.lua"))
    return 0;

  int opt;
  while ((opt = getopt_long(argc, const_cast<char *const*>(argv),
                            options, loptions, NULL)) != -1)
    {
      switch (opt)
        {
        case 'e':
          {
            int err = execute_lua_buf(L, optarg, strlen(optarg), optarg);
            if (err)
              fprintf(stderr, "Error executing cmdline statement\n");
            break;
          }
        default: break;
        }
    }

  // everything following the first non-option argument is considered an
  // argument for the Lua script and added to the 'arg' table
  // The script name itself is passed as arg[0].
  if (argc > optind)
    {
      if (!_create_table(L))
        {
          fprintf(stderr, "lua error: %s.\n", lua_tostring(L, -1));
          lua_pop(L, lua_gettop(L));
          return 0;
        }

      unsigned arg_idx = 0;
      for (int c = optind; c < argc; ++c, ++arg_idx)
        {
          lua_pushinteger(L, arg_idx);
          lua_pushstring(L, argv[c]);
          lua_settable(L, -3);
        }
      lua_setglobal(L, "arg");
    }

  printf("Ned: loading file: '%s'\n", argv[optind]);
  int e = luaL_dofile(L, argv[optind]);
  if (e)
    {
      // errors during Factory::create() are returned as a table with
      // { "msg" = message, "code" = return value }
      // extract error message from table and push it on the stack
      if (lua_istable(L, -1))
          lua_getfield(L, -1, "msg");

      fprintf(stderr, "lua error: %s.\n", lua_tostring(L, -1));
    }

  lua_gc(L, LUA_GCCOLLECT, 0);

  return 0;
}
