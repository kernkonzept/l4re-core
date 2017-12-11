-- vim:set ft=lua:
local require = require
local pairs = pairs
local setmetatable = setmetatable
local getmetatable = getmetatable
local error = error
local type = type

local _ENV = require "L4"
local string = require "string"
local table = require "table"

-- Add this alias, it sounds better for some cases
Env.user_factory = Env.mem_alloc;

-- L4 protocol constants
Proto = {
  Dataspace = 0x4000,
  Namespace = 0x4001,
  Goos      = 0x4003,
  Mem_alloc = 0x4004,
  Rm        = 0x4005,
  Event     = 0x4006,
  Inhibitor = 0x4007,
  Sigma0    = -6,
  Log       = -13,
  Scheduler = -14,
  Factory   = -15,
  Vm         = -16,
  Dma_space  = -17,
  Irq_sender = -18,
  Irq_muxer  = -19,
  Semaphore  = -20,
  Iommu      = -22,
  Ipc_gate  = 0,
}

-- L4 rights flags
Rights = {
  s   = 2,
  w   = 1,
  r   = 4,
  ro  = 4,
  rw  = 5,
  rws = 7,
}

-- Ldr flags
Ldr_flags = {
  eager_map    = 0x1, -- L4RE_AUX_LDR_FLAG_EAGER_MAP
  all_segs_cow = 0x2, -- L4RE_AUX_LDR_FLAG_ALL_SEGS_COW
  pinned_segs  = 0x4, -- L4RE_AUX_LDR_FLAG_PINNED_SEGS
}

-- Flags for dataspace allocation via user_factory
-- NOTE: keep constants in sync with l4re/include/mem_alloc
Mem_alloc_flags = {
  Continuous  = 1,
  Pinned      = 2,
  Super_pages = 4,
}

-- L4Re debug constants
Dbg = {
  Info       = 1,
  Warn       = 2,
  Boot       = 4,
  Server     = 0x10,
  Exceptions = 0x20,
  Cmd_line   = 0x40,
  Loader     = 0x80,
  Name_space = 0x400,
  All        = 0xffffffff,
}

-- Loader class, encapsulates a loader instance.
--  * A memory allocator
--  * A factory used for name-space creation (ns_fab)
--  * A factory used for region-map creation (rm_fab)
--  * A Factory used for log creation (log_fab)
--  * A Scheduler factory (sched_fab)

Loader = {};
Loader.__index = Loader;

Class = {};

function Class.check(obj, class)
  if not obj or getmetatable(obj) ~= class then
    error("method called with incompatible object", 3);
  end
end

function Loader.new(proto)
  local f = proto or {};

  do
    local lfab = f.loader or f.mem;
    f.log_fab = f.log_fab or lfab;
    f.ns_fab = f.ns_fab or lfab;
    f.rm_fab = f.rm_fab or lfab;
    f.sched_fab = f.sched_fab or lfab;
    f.factory = f.factory or Env.factory;
  end

  setmetatable(f, Loader);
  return f;
end

function mangle_class(n)
  local m = "N";
  for i in string.gmatch(n, "([^:%s]+)") do
    m = m .. #i .. i;
  end
  return m .. "E"; 
end

function cast(to, cap)
  if type(to) == 'number' then
    return __cast(to, cap)
  elseif type(to) == 'string' then
    return __cast(mangle_class(to), cap)
  end
  return nil
end

function get_cap_class(id)
  local t = type(id);
  if t == "number" then
    return _CAP_TYPES[id];
  elseif t == "string" then
    return _CAP_TYPES[mangle_class(id)];
  else
    return nil;
  end
end

local ns_class = get_cap_class("L4Re::Namespace");
if ns_class then
  ns_class.register = function (self, key, value, fab)
    if type(value) == "function" then
      value = value(self, key);
    end

    if value ~= nil then
      if type(value) ~= "table" then
        self:__register(key, value);
      elseif (fab) then
        self:__register(key, fab(value));
      end
    end
  end
  ns_class.r = ns_class.register;
else
  error("Could not find type information for L4Re::Namespace");
end

ns_class = nil;

function Loader.fill_namespace(ns, tmpl, fab)
  local function cns(value)
    return self:create_namespace(value, fab);
  end

  for k, v in pairs(tmpl) do
    ns:r(k, v, cns);
  end
end


function Loader:create_namespace(n, fab)
  Class.check(self, Loader);

  if type(n) ~= "table" then
    return n;
  end

  local ns_fab = fab or self.ns_fab;
  local ns = ns_fab:create(Proto.Namespace);
  self.fill_namespace(ns, n, ns_fab);
  return ns;
end



App_env = {}
App_env.__index = App_env;

function App_env.new(proto)
  local f = proto or {};

  f.loader = f.loader or default_loader;
  f.rm_fab = f.rm_fab or f.loader.rm_fab;
  f.factory = f.factory or f.loader.factory or Env.factory;
  --  f.scheduler = f.scheduler or f.loader.scheduler;

  f.mem = f.mem or f.loader.mem;

  if type(f.log) == "table" then
    f.log_args = f.log;
    f.log = nil;
  elseif type(f.log) == "function" then
    f.log = f.log()
    f.log_args = {}
  else
    f.log_args = {}
  end

  setmetatable(f, App_env);

  if type(f.ns) == "table" then
    f.ns = f.loader:create_namespace(f.ns, f.ns_fab);
  end

  return f;
end

function App_env:log()
  Class.check(self, App_env);
  if self.loader.log_fab == nil or self.loader.log_fab.create == nil then
    error ("Starting an application without valid log factory", 4);
  end
  return self.loader.log_fab:create(Proto.Log, table.unpack(self.log_args));
end

function App_env:start(...)
  Class.check(self, App_env);

  local function fa(a)
    return string.gsub(a, ".*/", "");
  end
  local old_log_tag = self.log_args[1];
  self.log_args[1] = self.log_args[1] or fa(...);
  local res = exec(self, ...);
  self.log_args[1] = old_log_tag;
  return res;
end

function App_env:set_ns(tmpl)
  Class.check(self, App_env);
  self.ns = Namespace.new(tmpl, self.ns_fab);
end

function App_env:set_loader_fab(fab)
  Class.check(self, App_env);
  self.log_fab = fab;
  self.ns_fab = fab;
  self.rm_fab = fab;
end

function App_env:set_mem_alloc(mem)
  Class.check(self, App_env);
  self.mem = mem;
end

function Loader:startv(env, ...)
  Class.check(self, Loader);

  local caps = env.caps or {};

  if (type(caps) == "table") then
    for k, v in pairs(caps) do
      if type(v) == "table" and getmetatable(v) == nil then
        caps[k] = self:create_namespace(v)
      end
    end

    caps.rom = caps.rom or Env.rom:m("r");
  end

  env.loader = self;
  env.caps = caps;
  env.l4re_dbg = env.l4re_dbg or Dbg.Warn;
  local e = App_env.new(env);
  return e:start(...);
end

-- Create a new IPC gate for a client-server connection
function Loader:new_channel()
  return self.factory:create(Proto.Ipc_gate);
end

function Loader.split_args(cmd, posix_env)
  local a = {};
  local i = 1;
  for w in string.gmatch(cmd, "[^%s]+") do
    a[i] = w;
    i = i + 1;
  end
  a[i] = posix_env;
  return table.unpack(a);
end

function Loader:start(env, cmd, posix_env)
  Class.check(self, Loader);
  return self:startv(env, self.split_args(cmd, posix_env));
end

default_loader = Loader.new({factory = Env.factory, mem = Env.mem_alloc});

return _ENV
