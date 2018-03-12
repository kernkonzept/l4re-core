PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		 = ned

SRC_CC          := remote_mem.cc app_model.cc app_task.cc main.cc \
                   lua.cc lua_env.cc lua_ns.cc lua_cap.cc \
	           lua_exec.cc lua_factory.cc lua_info.cc server.cc \
		   lua_platform_control.cc
SRC_DATA        := ned.lua

REQUIRES_LIBS   := libloader l4re-util l4re lua++ libpthread cxx_libc_io cxx_io

ifeq (1,0)
DEFINES         += -DUSE_READLINE
REQUIRES_LIBS   += readline libc_be_file_stdin
endif

include $(L4DIR)/mk/prog.mk

