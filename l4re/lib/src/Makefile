PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../../..

TARGET	      = lib4re.a lib4re.so
REQUIRES_LIBS = l4util l4sys
PC_EXTRA      = Link_Libs= %{static:-l4re}

SRC_CC	= namespace.cc dataspace.cc rm.cc mem_alloc.cc \
	  log.cc event.cc video/goos.cc
SRC_C_lib4re.a   := env.c

include $(L4DIR)/mk/lib.mk

