PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../../..

TARGET	       = lib4re-util.a lib4re-util.so
SRC_CC        := cap_alloc.cc debug.cc name_space_svr.cc dataspace_svr.cc \
                 goos_fb.cc kumem_alloc.cc
PC_FILENAME   := l4re-util
REQUIRES_LIBS := l4re

include $(L4DIR)/mk/lib.mk
