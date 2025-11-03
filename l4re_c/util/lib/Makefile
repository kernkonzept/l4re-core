PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET	      = lib4re-c-util.ofl lib4re-c-util.p.a
SRC_CC       := cap_alloc.cc video/goos_fb.cc kumem_alloc.cc
PC_FILENAME   = l4re_c-util
PC_LIBS      := %{-link-libc:%{shared:--whole-archive -l4re-c-util.p --no-whole-archive;:lib4re-c-util.ofl}}
PC_LIBS_PIC  :=

CXXFLAGS += -fno-rtti -fno-exceptions

include $(L4DIR)/mk/lib.mk
