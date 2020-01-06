PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		 = ned

SRC_CC          := remote_mem.cc app_model.cc app_task.cc main.cc \
                   lua.cc lua_env.cc lua_ns.cc lua_cap.cc \
                   lua_exec.cc lua_factory.cc lua_info.cc server.cc \
                   lua_platform_control.cc lua_debug_obj.cc
SRC_DATA        := ned.lua

SRC_CC          += lua_sleep.cc

REQUIRES_LIBS   := libloader l4re-util l4re lua++ libpthread cxx_libc_io cxx_io

include $(L4DIR)/mk/prog.mk
