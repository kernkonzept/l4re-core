-- vim:set ft=lua:
local require = require
local ipairs = ipairs
local pairs = pairs
local setmetatable = setmetatable
local getmetatable = getmetatable
local error = error
local type = type
local tonumber = tonumber

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
  Semaphore  = -20,
  Iommu      = -22,
  Ipc_gate  = 0,
}

-- L4 rights flags
Rights = {
  s   = 2,
  w   = 1,
  r   = 4,
  d   = 8,
  n   = 16,
  c   = 32,
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
--  * A factory used for log creation (log_fab)
--  * A factory used for sched_proxy creation (sched_fab)

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
    if type(value) == "string" then
      local res = Env
      for i in string.gmatch(value, "([^/]+)") do
        if type(res) == "table" then
          res = res[i]
        elseif res then
          res = res:query(i)()
        else
          break
        end
      end
      if res == nil then
        error("Could not resolve: '" .. value .. "'", 5)
      end
      value = res
    end

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

function Loader:fill_namespace(ns, tmpl, fab)
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
  self:fill_namespace(ns, n, ns_fab);
  return ns;
end



App_env = {}
App_env.__index = App_env;

function App_env.new(proto)
  local f = proto or {};

  f.loader = f.loader or default_loader;
  f.rm_fab = f.rm_fab or f.loader.rm_fab;
  f.factory = f.factory or f.loader.factory or Env.factory;
  f.dbg_events = f.dbg_events or f.loader.dbg_events or Env.dbg_events;
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

    local defcaps = self.default_caps or { rom = Env.rom:m("r") }

    for k, v in pairs(defcaps) do
      caps[k] = caps[k] or v
    end
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

function Loader:create_sched_proxy(props)
  Class.check(self, Loader);

  local sched_fab = props.fab or self.sched_fab;
  local cset = props.cpus or Cpu_set:new();

  local prio_offset = props.prio_offset or 0
  local prio_limit = props.prio_limit or prio_offset + 10
  return sched_fab:create(Proto.Scheduler, prio_limit,
                          prio_offset, cset:factory_args());
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

function Loader:start_backtracer(opts)
  local o = opts or {};
  local prog = o.prog or "rom/backtracer";
  local bt = self:new_channel();
  self:startv({ caps = { backtracer = bt:svr() },
                log = self.log_fab:create(Proto.Log, "backtracer", "n", "show"),
                scheduler = Env.user_factory:create(Proto.Scheduler,
                                                    0xff, 0xfc)
              }, prog, table.unpack(o.args or {}));
  -- After starting it such that it does not have itself set as a
  -- backtracer
  self.dbg_events = bt;
end

default_loader = Loader.new({factory = Env.factory,
                             mem = Env.mem_alloc,
                             dbg_events = Env.dbg_events});


Cpu_set = {}
Cpu_set.__index = Cpu_set

function Cpu_set:__tostring()
  if self.all then
     return "{ all }"
  else
     local s = "{"
     local sep = ""

     local max_cpu = 0
     for n, _ in pairs(self.set) do
        if n > max_cpu then max_cpu = n end
     end
     for n = 0, max_cpu do
        if self.set[n] then
           s = s .. sep .. n
           sep = ", "
        end
     end
     return s .. "}"
  end
end

function Cpu_set:new(cpus)
  local obj = { all = true, set = {} }
  setmetatable(obj, self)

  if cpus then
    obj.all = false
    if type(cpus) == "table" then
      for _, n in ipairs(cpus) do
        obj:add(n, 1)
      end
    else
      obj:add(cpus, 1)
    end
  end

  return obj
end

-- Add a single or range of CPUs
function Cpu_set:add(cpu, lvl)
  if self.all then
    return
  end

  lvl = lvl or 0
  arg_type = type(cpu)
  if arg_type == "number" then
    self.set[cpu] = true
  elseif arg_type == "string" then
    sfrom, sto = string.match(cpu, "(%d+)-(%d+)")
    from = tonumber(sfrom)
    to = tonumber(sto)
    if from and to and from <= to then
      for i = tonumber(from), tonumber(to) do
        self.set[i] = true
      end
    else
      error("Invalid CPU range \"" .. cpu .. "\"", lvl + 2)
    end
  else
    error("Invalid argument type: " .. arg_type, lvl + 2)
  end
end

-- CPU set union - return all CPUs from `self` and `other`
function Cpu_set:__bor(other)
  local ret = {
    all = self.all or other.all,
    set = {}
  }
  setmetatable(ret, Cpu_set)

  if not ret.all then
    for n, _ in pairs(self.set) do
      ret.set[n] = true
    end
    for n, _ in pairs(other.set) do
      ret.set[n] = true
    end
  end

  return ret
end

-- CPU set intersection - return only CPUs that are common to `self` and `other`
function Cpu_set:__band(other)
  local ret = {
    all = self.all and other.all,
    set = {}
  }
  setmetatable(ret, Cpu_set)

  if ret.all then
  elseif self.all then
    for n, _ in pairs(other.set) do
      ret.set[n] = true
    end
  elseif other.all then
    for n, _ in pairs(self.set) do
      ret.set[n] = true
    end
  else
    ret.set = {}
    for n, _ in pairs(other.set) do
      if self.set[n] then
        ret.set[n] = true
      end
    end
  end

  return ret
end

-- Return cpu mask arguments for user factory scheduler proxy
function Cpu_set:factory_args()
  if self.all then
    return  -- no CPU mask words imply all CPUs
  end

  local cpu_masks = {}
  local mword_bits = Info.mword_bits()
  local max_cpu = 0
  for n, _ in pairs(self.set) do
    if n > max_cpu then max_cpu = n end
  end

  for word = 0, max_cpu // mword_bits do
    local mask = 0
    for i = 0, mword_bits - 1 do
      if self.set[word * mword_bits + i] then
        mask = mask | (1 << i)
      end
    end
    table.insert(cpu_masks, mask)
  end

  return table.unpack(cpu_masks)
end

return _ENV
