PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET      = libc_be_minimal_log_io.ofl
PC_FILENAME = libc_be_minimal_log_io
PC_LIBS     = %{-link-libc:%{shared:;:libc_be_minimal_log_io.ofl}}
SRC_CC      = be.cc
CXXFLAGS    = -fno-rtti -fno-exceptions

include $(L4DIR)/mk/lib.mk

CPPFLAGS-$(CONFIG_L4_LIBC_UCLIBC) += -DNOT_IN_libc -include libc-symbols.h
