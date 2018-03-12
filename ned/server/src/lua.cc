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

static char const *const options = "+ie:";
static struct option const loptions[] =
  {
    { "interactive", 0, NULL, 'i' },
    { "noexit", 0, NULL, 1 },
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

int lua(int argc, char const *const *argv)
{
  printf("Ned says: Hi World!\n");

  bool interactive = false;
  bool noexit = false;

  if (argc < 2)
    interactive = true;

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
        case 'i': interactive = true; break;
        case 1: noexit = true; break;
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

  if (optind >= argc)
    interactive = true;

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

  if (!interactive)
    return 0;

#ifndef USE_READLINE
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

      if (luaL_loadbuffer(L, cmd, strlen(cmd), "argument"))
        {
          fprintf(stderr, "lua couldn't parse '%s': %s.\n",
                  cmd, lua_tostring(L, -1));
          lua_pop(L, 1);
        }
      else
        {
          if (lua_pcall(L, 0, 1, 0))
            {
              fprintf(stderr, "lua couldn't execute '%s': %s.\n",
                      cmd, lua_tostring(L, -1));
              lua_pop(L, 1);
            }
          else
            lua_pop(L, lua_gettop(L));
        }
    }
#endif

  return 0;
}
