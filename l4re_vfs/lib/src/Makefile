PKGDIR    ?= ../..
L4DIR     ?= $(PKGDIR)/../../..

PC_FILENAME = libl4re-vfs
TARGET	    = libl4re-vfs.o.a
PC_LIBS     = %{-link-libc:%{shared:;:libl4re-vfs.o.a}}
SRC_CC	    = vfs.cc
REQUIRES_LIBS = l4re-util

include $(L4DIR)/mk/lib.mk

CFLAGS   += -fvisibility=hidden
CXXFLAGS += -fvisibility=hidden
# No exception information as unwinder code might use malloc and friends
DEFINES += -DNOT_IN_libc -DL4_NO_RTTI
CXXFLAGS += -include libc-symbols.h -fno-exceptions -fno-rtti
