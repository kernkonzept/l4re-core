PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET        = lua
SRC_C         = lua.c
REQUIRES_LIBS = lua libc_support_misc libc_be_fs_noop libc_be_sig_noop

vpath %.c $(PKGDIR)/lib/contrib/src

include $(L4DIR)/mk/prog.mk
