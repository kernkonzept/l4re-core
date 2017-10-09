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
#include <l4/re/util/br_manager>
#include <l4/re/util/object_registry>
#include <l4/ned/cmd_control>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <getopt.h>

#ifdef USE_READLINE
#include <readline/history.h>
#include <readline/readline.h>
#endif

#include "lua_cap.h"
#include "lua.h"

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
// { LUA_IOLIBNAME, luaopen_io },
  { LUA_STRLIBNAME, luaopen_string },
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_DBLIBNAME, luaopen_debug},
  { NULL, NULL }
};

static char const *const options = "+ie:c:";
static struct option const loptions[] =
  {
    { "interactive", 0, NULL, 'i' },
    { "noexit", 0, NULL, 1 },
    { "execute", 1, NULL, 'e' },
    { "cmdcap", 1, NULL, 'c' },
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

static void
run_interactive(lua_State *lua, bool noexit)
{
#ifndef USE_READLINE
  (void)noexit;
  (void)lua;
  fprintf(stderr, "Ned: Interactive mode not compiled in.\n");
#else
  printf("Ned: Interactive mode.\n");
  const char *cmd;
  for (;;)
    {
      cmd = readline((char *)"Ned: ");

      if (0)
        printf("INPUT: %s\n", cmd);

      if (!cmd)
        {
          if (noexit)
            continue;
          break;
        }

      if (luaL_loadbuffer(lua, cmd, strlen(cmd), "argument"))
        {
          fprintf(stderr, "lua couldn't parse '%s': %s.\n",
                  cmd, lua_tostring(lua, -1));
          lua_pop(lua, 1);
        }
      else
        {
          if (lua_pcall(lua, 0, 1, 0))
            {
              fprintf(stderr, "lua couldn't execute '%s': %s.\n",
                      cmd, lua_tostring(lua, -1));
              lua_pop(lua, 1);
            }
          else
            lua_pop(lua, lua_gettop(lua));
        }
    }
#endif
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
        fprintf(stderr, "lua couldn't parse '%.*s': %s.\n", (int)cmd.length - 1,
                cmd.data, lua_tostring(_lua, -1));
        lua_pop(_lua, 1);

        return -L4_EINVAL;
      }

    if (lua_pcall(_lua, 0, 1, 0))
      {
        fprintf(stderr, "lua couldn't execute '%.*s': %s.\n", (int)cmd.length,
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

int lua(int argc, char const *const *argv)
{
  printf("Ned says: Hi World!\n");

  enum Mode { None, Interactive, Cmd_channel };
  Mode mode = None;
  bool noexit = false;

  if (argc < 2)
    mode = Interactive;

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
  char const *cmd_client = 0;
  while ((opt = getopt_long(argc, const_cast<char *const*>(argv),
                            options, loptions, NULL)) != -1)
    {
      switch (opt)
        {
        case 'i':
          if (mode == None)
            mode = Interactive;
          else
            fprintf(stderr, "Must set either interactive or command channel mode."
                            " Interactive switch ignored.\n");
          break;
        case 1: noexit = true; break;
        case 'e':
          {
            int err = execute_lua_buf(L, optarg, strlen(optarg), optarg);
            if (err)
              fprintf(stderr, "Error executing cmdline statement\n");
            break;
          }
        case 'c':
          if (mode == Interactive)
            fprintf(stderr, "Must set either interactive or command channel mode."
                            " command channel capability ignored.\n");
          else
            {
              cmd_client = optarg;
              mode = Cmd_channel;
            }
          break;
        default: break;
        }
    }

  if (optind >= argc && mode == None)
    mode = Interactive;

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
    fprintf(stderr, "lua error: %s.\n", lua_tostring(L, -1));

  lua_gc(L, LUA_GCCOLLECT, 0);

  if (mode == Interactive)
    run_interactive(L, noexit);
  else if (mode == Cmd_channel)
    {
      L4Re::Util::Registry_server<L4Re::Util::Br_manager_timeout_hooks> server;
      Command_dispatcher cmd_dispatch(L);

      server.registry()->register_obj(&cmd_dispatch, cmd_client);

      server.loop();
    }

  return 0;
}
