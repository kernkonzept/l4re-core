PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../../..

TARGET	       = lib4re-util.ofl lib4re-util.p.a
SRC_CC        := cap_alloc.cc debug.cc kumem_alloc.cc
PC_FILENAME   := l4re-util
PC_LIBS       := %{-link-libc:%{shared:--whole-archive -l4re-util.p --no-whole-archive;:lib4re-util.ofl}}
PC_LIBS_PIC   :=

PICFLAGS += -DSHARED=1
CXXFLAGS += -fno-rtti -fno-exceptions

include $(L4DIR)/mk/lib.mk
