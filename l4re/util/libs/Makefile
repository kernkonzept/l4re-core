PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../../..

TARGET	       = lib4re-util.a lib4re-util.so
SRC_CC        := cap_alloc.cc debug.cc dataspace_svr.cc \
                 kumem_alloc.cc
PC_FILENAME   := l4re-util
REQUIRES_LIBS := l4re

PICFLAGS += -DSHARED=1
CXXFLAGS       += -fno-rtti -fno-exceptions

include $(L4DIR)/mk/lib.mk
