/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#include "app_task.h"
#include "app_model.h"
#include "debug.h"

#include <l4/cxx/auto_ptr>
#include <l4/cxx/ref_ptr>
#include <l4/libloader/elf>
#include <l4/util/bitops.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <pthread-l4.h>
#include "lua.h"
#include "lua_cap.h"
#include "server.h"

using L4Re::chksys;

inline void *operator new (size_t, void *p) throw() { return p; }
namespace Lua { namespace {

struct Obs_iface : L4::Kobject_0t<Obs_iface>
{
  L4_INLINE_RPC(long, wait, (l4_cap_idx_t thread, l4_addr_t task));
  typedef L4::Typeid::Rpcs<wait_t> Rpcs;
};

class Observer :
  public L4::Epiface_t<Observer, Obs_iface, Ned::Server_object>
{
public:
  long op_wait(Obs_iface::Rights, l4_cap_idx_t thread, l4_addr_t task);
};

static Observer *observer;

long
Observer::op_wait(Obs_iface::Rights, l4_cap_idx_t thread, l4_addr_t task)
{
  App_task *t = (App_task*)task;

  if (t->state() == App_task::Zombie)
    return 0;

  t->observer(thread);
  return -L4_ENOREPLY;
}

class Am : public Rmt_app_model
{
private:
  lua_State *_lua;
  int _argc;
  int _env_idx;
  int _cfg_idx;
  int _arg_idx;

  L4::Cap<L4::Factory> _rm_fab;

  l4_umword_t _cfg_integer(char const *f, l4_umword_t def = 0)
  {
    l4_umword_t r = def;
    lua_getfield(_lua, _cfg_idx, f);
    if (lua_isnumber(_lua, -1))
      r = lua_tointeger(_lua, -1);

    lua_pop(_lua, 1);
    return r;
  }

  void _cfg_cap(char const *f, l4_fpage_t *r)
  {
    lua_getfield(_lua, _cfg_idx, f);
    while (lua_isfunction(_lua, -1))
      {
	lua_pushvalue(_lua, _cfg_idx);
	lua_call(_lua, 1, 1);
      }

    if (!lua_isnil(_lua, -1))
      {
	Cap *c = Lua::check_cap(_lua, -1);
	*r = c->cap<void>().fpage(c->rights());
      }
    lua_pop(_lua, 1);
  }

public:

  explicit Am(lua_State *l)
  : Rmt_app_model(), _lua(l), _argc(lua_gettop(l)), _env_idx(0), _cfg_idx(1),
    _arg_idx(2)
  {
    if (_argc > 2 && lua_type(_lua, _argc) == LUA_TTABLE)
      _env_idx = _argc;

    if (_env_idx)
      --_argc;
  }

  l4_cap_idx_t push_initial_caps(l4_cap_idx_t start)
  {
    lua_getfield(_lua, _cfg_idx, "caps");
    int tab = lua_gettop(_lua);

    if (lua_isnil(_lua, tab))
      {
	lua_pop(_lua, 1);
        return start;
      }

    lua_pushnil(_lua);
    while (lua_next(_lua, tab))
      {
	char const *r = luaL_checkstring(_lua, -2);
        if (!l4re_env_cap_entry_t::is_valid_name(r))
          luaL_error(_lua, "Capability name '%s' too long", r);
	while (lua_isfunction(_lua, -1))
	  {
	    lua_pushvalue(_lua, tab);
	    lua_call(_lua, 1, 1);
	  }

	if (!lua_isnil(_lua, -1) && lua_touserdata(_lua, -1))
	  {
	    Lua::check_cap(_lua, -1);
	    _stack.push(l4re_env_cap_entry_t(r, get_initial_cap(r, &start)));
	  }
	lua_pop(_lua, 1);
      }
    lua_pop(_lua, 1);
    return start;
  }

  void map_initial_caps(L4::Cap<L4::Task> task, l4_cap_idx_t start)
  {
    lua_getfield(_lua, _cfg_idx, "caps");
    int tab = lua_gettop(_lua);

    if (lua_isnil(_lua, tab))
      {
	lua_pop(_lua, 1);
        return;
      }

    lua_pushnil(_lua);
    while (lua_next(_lua, tab))
      {
	char const *r = luaL_checkstring(_lua, -2);
	while (lua_isfunction(_lua, -1))
	  {
	    lua_pushvalue(_lua, tab);
	    lua_call(_lua, 1, 1);
	  }

	if (!lua_isnil(_lua, -1) && lua_touserdata(_lua, -1))
	  {
	    Cap *c = Lua::check_cap(_lua, -1);
	    auto idx = get_initial_cap(r, &start);
	    chksys(task->map(L4Re::This_task,
	                     c->cap<void>().fpage(c->rights()),
	                     L4::Cap<void>(idx).snd_base() | c->ext_rights()));
	  }
	lua_pop(_lua, 1);
      }
    lua_pop(_lua, 1);
  }

  void launch_loader()
  {
    char const *kernel = "rom/l4re";
    lua_getfield(_lua, _cfg_idx, "l4re_loader");
    if (lua_isstring(_lua, -1))
      kernel = lua_tostring(_lua, -1);

    typedef Ldr::Elf_loader<Am, Dbg> Loader;

    Dbg ldr(Dbg::Loader, "ldr");
    Loader _l;

    _l.launch(this, kernel, ldr);

    lua_pop(_lua, 1);
  }

  void parse_cfg()
  {
    prog_info()->mem_alloc = L4Re::Env::env()->mem_alloc().fpage();
    prog_info()->log = L4Re::Env::env()->log().fpage();
    prog_info()->factory = L4Re::Env::env()->factory().fpage();
    prog_info()->scheduler = L4Re::Env::env()->scheduler().fpage();
    //  parser.scheduler_cap.set_fpage(&am.prog_info()->scheduler);

    prog_info()->ldr_flags = 0;
    prog_info()->l4re_dbg = 0;

    if (!_cfg_idx)
      return;

    prog_info()->ldr_flags = _cfg_integer("ldr_flags", prog_info()->ldr_flags);
    prog_info()->l4re_dbg = _cfg_integer("l4re_dbg", prog_info()->l4re_dbg);

    _cfg_cap("log", &prog_info()->log);
    _cfg_cap("mem", &prog_info()->mem_alloc);
    _cfg_cap("factory", &prog_info()->factory);
    _cfg_cap("scheduler", &prog_info()->scheduler);

    l4_fpage_t fab = prog_info()->mem_alloc;
    _cfg_cap("rm_fab", &fab);
    _rm_fab = L4::Cap<L4::Factory>(fab.raw);
  }

  L4::Cap<L4::Factory> rm_fab() const { return _rm_fab; }

  void set_task(App_task *t) { _task = t; }

  void push_argv_strings()
  {
    argv.a0 = 0;
    for (int i = _arg_idx; i <= _argc; ++i)
      {
        if (lua_isnil(_lua, i))
          continue;

	size_t l;
	char const *r = luaL_checklstring(_lua, i, &l);
	argv.al = _stack.push_str(r, l);
	if (argv.a0 == 0)
	  argv.a0 = argv.al;
      }
  }

  void push_env_strings()
  {
    if (!_env_idx)
      return;

    lua_pushnil(_lua);
    bool _f = true;
    while (lua_next(_lua, _env_idx))
      {
	size_t kl;
	char const *k = luaL_checklstring(_lua, -2, &kl);
	size_t vl;
	char const *v = luaL_checklstring(_lua, -1, &vl);

	_stack.push_str(v, vl);
	_stack.push('=');
	envp.al = _stack.push_object(k, kl);
	if (_f)
	  {
	    envp.a0 = envp.al;
	    _f = false;
	  }
	lua_pop(_lua, 1);
      }
  }
};


static char const *const APP_TASK_TYPE = "L4_NED_APP_TASK";
typedef cxx::Ref_ptr<App_task> App_ptr;

static
App_ptr &check_at(lua_State *l, int i)
{
  App_ptr *t = (App_ptr*)luaL_checkudata(l, i, APP_TASK_TYPE);
  return *t;
}

static int __task_state(lua_State *l)
{
  App_ptr t = check_at(l, 1);

  if (!t)
    {
      lua_pushnil(l);
      return 1;
    }

  switch (t->state())
    {
    case App_task::Initializing:
      lua_pushstring(l, "initializing");
      break;

    case App_task::Running:
      lua_pushstring(l, "running");
      break;

    case App_task::Zombie:
      lua_pushstring(l, "zombie");
      break;

    default:
      lua_pushstring(l, "nan");
      break;
    }

  return 1;
}

static int __task_exit_code(lua_State *l)
{
  App_ptr t = check_at(l, 1);
  if (!t)
    lua_pushnil(l);
  else
    lua_pushinteger(l, t->exit_code());

  return 1;
}

static int __task_wait(lua_State *l)
{
  App_ptr &t = check_at(l, 1);

  if (!t)
    {
      lua_pushnil(l);
      return 1;
    }

  L4::cap_cast<Obs_iface>(observer->obj_cap())->wait(pthread_l4_cap(pthread_self()), l4_addr_t(t.get()));
  lua_pushinteger(l, t->exit_code());

  t = 0; // zap task
  return 1;
}

static int __task_kill(lua_State *l)
{
  App_ptr &t = check_at(l, 1);

  if (!t)
    {
      lua_pushnil(l);
      return 1;
    }

  if (t->state() == App_task::Zombie)
    {
      lua_pushinteger(l, t->exit_code());
      t = 0; // zap task
      return 1;
    }

  t->terminate();
  t = 0; // kill task
  lua_pushstring(l, "killed");

  return 1;
}

static int __task_gc(lua_State *l)
{
  App_ptr &t = check_at(l, 1);
  t = 0; // drop reference to task
  return 0;
}

static int __task_eq(lua_State *l)
{
  App_ptr const &t1 = check_at(l, 1);
  App_ptr const &t2 = check_at(l, 2);
  lua_pushboolean(l, t1 == t2);
  return 1;
}

static int __task_lt(lua_State *l)
{
  App_ptr const &t1 = check_at(l, 1);
  App_ptr const &t2 = check_at(l, 2);
  lua_pushboolean(l, t1 < t2);
  return 1;
}

static int __task_le(lua_State *l)
{
  App_ptr const &t1 = check_at(l, 1);
  App_ptr const &t2 = check_at(l, 2);
  lua_pushboolean(l, t1 <= t2);
  return 1;
}

static const luaL_Reg _task_ops[] = {
    { "state", __task_state },
    { "exit_code", __task_exit_code },
    { "wait", __task_wait },
    { "kill", __task_kill },
    { NULL, NULL }
};


static int exec(lua_State *l)
{
  try {

  Am am(l);
  am.parse_cfg();

  App_ptr app_task(new App_task(Ned::server->registry(), am.rm_fab()));

  if (!app_task)
    {
      Err().printf("could not allocate task control block\n");
      return 0;
    }


  am.set_task(app_task.get());

  app_task->running();

  am.launch_loader();

  App_ptr *at = new (lua_newuserdata(l, sizeof(App_ptr))) App_ptr();
  *at = app_task;

  luaL_newmetatable(l, APP_TASK_TYPE);
  lua_setmetatable(l, -2);

  return 1;
  } catch (L4::Runtime_error const &e) {
    luaL_error(l, "could not create process: %s (%s: %d)", e.str(), e.extra_str(), e.err_no());
  }

  return 0;
}
#if 0
void do_some_exc_tests()
{
  char _ta[333];
  char const *const cont = "Das ist ein lustiger test";

  for (unsigned i = 0; i < strlen(cont); ++i)
    _ta[i] = cont[i];

  volatile int *x = (int*)0x500;
//printf("Test Exc\n");
  int y = *x;
}
#endif


static const luaL_Reg _task_meta_ops[] = {
    { "__gc", __task_gc },
    { "__eq", __task_eq },
    { "__lt", __task_lt },
    { "__le", __task_le },
    { NULL, NULL }
};

class Lib_exec : public Lib
{
public:
  Lib_exec() : Lib(P_env) {}
  void init(lua_State *l)
  {
    static const luaL_Reg _ops[] =
    {
      { "exec", exec },
      { NULL, NULL }
    };
    Lua::lua_require_module(l, "L4");
    luaL_setfuncs(l, _ops, 0);

    if (luaL_newmetatable(l, APP_TASK_TYPE))
      {
	lua_newtable(l);
	luaL_setfuncs(l, _task_ops, 0);
	lua_setfield(l, -2, "__index");
	luaL_setfuncs(l, _task_meta_ops, 0);
      }
    lua_pop(l, 2);

    observer = new Observer();
    Ned::server->registry()->register_obj(observer);
  }
};

static Lib_exec __lib;

}}

