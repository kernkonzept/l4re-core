PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET      = libc_be_minimal_log_io.a
PC_FILENAME = libc_be_minimal_log_io
PC_EXTRA    = Link_Libs= %{static|static-pie:-lc_be_minimal_log_io}
SRC_CC      = be.cc
CXXFLAGS    = -DNOT_IN_libc -include libc-symbols.h -DL4_NO_RTTI -fno-rtti -fno-exceptions
REQUIRES_LIBS := l4re

include $(L4DIR)/mk/lib.mk
