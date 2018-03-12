local require = require
local pairs = pairs
local collectgarbage = collectgarbage

local _ENV = {};
local L4 = require("L4");

-- The defaults for loading the services
loader = L4.default_loader;

local rom = L4.Env.rom;

local bus_rtc;
local bus_usbhid;
local bus_gfx;
local bus_ps2;
local rtc_if;
local hid_ev;


-- start the IO server, with the given arguments.
-- @return A capability table, containing all the
--         capabilities to the buses specified in the config files
--         of IO.
function io(busses, ...)
  bus_rtc    = loader:new_channel();
  bus_usbhid = loader:new_channel();
  bus_gfx    = loader:new_channel();
  bus_ps2    = loader:new_channel();

  local io_caps = {
    rom = rom;
    icu = L4.Env.icu;
    sigma0 = L4.cast(L4.Proto.Factory, L4.Env.sigma0):create(L4.Proto.Sigma0);

    rtc_bus    = bus_rtc:svr();
    bus_usbhid = bus_usbhid:svr();
    bus        = bus_ps2:svr();
    fb_bus     = bus_gfx:svr();
  };

  for k, v in pairs(busses) do
    io_caps[k] = v:svr();
  end

  loader:startv({
    caps = io_caps,
    log = {"io", "red"}, l4re_dbg = L4.Dbg.Warn + L4.Dbg.Loader,
    scheduler = L4.Env.user_factory:create(L4.Proto.Scheduler, 0x90, 0x80)
  }, "rom/io", ...);
  return io_caps;
end

-- Start the RTC server, used on x86 for l4linux
-- @return The namespace capability of RTC, containing 'rtc'
--         as service cap. 
function rtc(...)

  rtc_if = loader:new_channel();

  local rtc_caps = {
    rom  = rom;
    vbus = bus_rtc;
    rtc  = rtc_if:svr();
  };

  -- drop bus_rtc reference it is no longer needed in ned
  bus_rtc = nil;

  loader:startv({caps = rtc_caps, log = {"rtc", "green"}}, "rom/rtc", ...);
  return rtc_if;
end


-- Start the HID server.
-- @return A capability to the hid name space.
--
-- Also saves the name-space capability for later use e.g. by gui
function hid(...)
  -- do not start the HID Linux on AMD64
  if L4.Info.arch() == "amd64" then
    return;
  end
  hid_ev = loader:create_namespace({ ev_irq = "ph", ev_buf="ph" });

  local hid_caps = {
    rom  = rom;
    vbus = bus_usbhid;
    rtc  = rtc_if;
    ev   = hid_ev:m("rws");
  };

  bus_usbhid = nil;
  rtc_if = nil;

  loader:startv({
    caps = hid_caps,
    log = {"hid", "m"},
    scheduler = L4.Env.user_factory:create(L4.Proto.Scheduler, 0x80, 0x70)
  }, "rom/vmlinuzusbv", "init=none", "showpfexc=0", "mem=12M",
     "l4x_cpus=4", "l4ser.vkey_enable=1", "console=ttyLv0", ...);

  return hid_caps;
end


-- Start the GUI server
-- @return The capability to the GUI-server name space.
function gui(svc_caps, fb, ...)

  local gui_caps = {
    rom  = rom;
    vbus = bus_ps2;
    fb   = fb;
  };

  bus_ps2 = nil;

  for k, v in pairs(svc_caps) do
    gui_caps[k] = v:svr();
  end

  local input_drv = "";
  if not hid_ev then -- run mag with libinput
    input_drv = "input-libinput";
  else
    gui_caps.ev = hid_ev;
    hid_ev = nil;
    input_drv = "input-lxdd";
  end

  loader:start({
    caps = gui_caps,
    log = { "gui", "yellow" },
    scheduler =  L4.Env.user_factory:create(L4.Proto.Scheduler, 0x78, 0x70)
  },
  "rom/mag " .. input_drv .." client_fb mag_client", ...);

  return gui_caps;
end


-- Get the Hw frame-buffer capability.
-- This function either uses the 'vesa' capability provided by moe
-- or starts fb-drv if 'vesa' is not available.
-- @return The capability to the Hw Goos object.
function fb(...)
  local fb = L4.Env.vesa;
  if fb then
    return fb;
  end

  local fb_caps = {
    rom  = rom;
    vbus = bus_gfx;
    fb   = loader:new_channel():svr(); -- gfx
  };

  bus_gfx = nil;

  loader:startv({caps = fb_caps, log = {"fb-drv","blue"}}, "rom/fb-drv", "-m", ...);

  return fb_caps.fb:m("rw"); --client
end

function cleanup()
  collectgarbage();
end

return _ENV;
