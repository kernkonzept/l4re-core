PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../../..

TARGET	      = lib4re.ofl lib4re.p.a
REQUIRES_LIBS = l4util l4sys
PC_LIBS       = %{-link-libc:%{shared:--whole-archive -l4re.p --no-whole-archive;:lib4re.ofl}}
PC_LIBS_PIC   =

SRC_CC	= namespace.cc dataspace.cc rm.cc mem_alloc.cc \
	  log.cc event.cc video/goos.cc
SRC_C_lib4re.ofl := env.c

CXXFLAGS += -fno-rtti -fno-exceptions

include $(L4DIR)/mk/lib.mk

