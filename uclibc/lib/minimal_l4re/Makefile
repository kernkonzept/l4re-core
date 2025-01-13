# vi:ft=make
PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../../..

TARGET          := libc_minimal_l4re.a
PC_FILENAME     := libc_minimal_l4re
define PC_EXTRA
Link_Libs= %{static|static-pie:-lc_minimal_l4re}
endef

DEFINES         := -DL4_MINIMAL_LIBC -DL4_NO_RTTI=1
SRC_CC          := _exit.cc
LDFLAGS         = --link-libc

vpath _exit.cc $(SRC_DIR)/../uclibc

include $(L4DIR)/mk/lib.mk

# Gather all libraries that shall be linked into libc. We need to search them
# explicitly because no linker is involved here. Only take the first hit, like
# ld would do.
LINK_LIBC_LIBNAMES = $(call BID_PKG_CONFIG,$(OBJ_BASE),--libs -- --link-libc,$(REQUIRES_LIBS))
LINK_LIBC_LIBS = $(foreach l,$(LINK_LIBC_LIBNAMES),$(firstword $(wildcard $(addsuffix /$(l),$(L4LIBDIR)))))

OBJS_libc_minimal_l4re.a += $(LINK_LIBC_LIBS)
