PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET      = libc_be_sig.ofl libc_be_sig.p.a
PC_FILENAME = libc_be_sig
PC_LIBS     = %{-link-libc:%{shared:--whole-archive -lc_be_sig.p --no-whole-archive;:libc_be_sig.ofl}}
PC_LIBS_PIC =
SRC_CC      = sig.cc
CXXFLAGS    = -fno-rtti -fno-exceptions
DEFINES     = -DL4_NO_RTTI

include $(L4DIR)/mk/lib.mk
