# vi:ft=make
PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../../..

TARGET          := libc_be_minimal_l4re.a
PC_FILENAME     := libc_be_minimal_l4re
define PC_EXTRA
Link_Libs= %{static|static-pie:-lc_be_minimal_l4re}
endef

DEFINES         := -DL4_MINIMAL_LIBC
SRC_CC          := _exit.cc
REQUIRES_LIBS   := l4re

vpath _exit.cc $(SRC_DIR)/../../../uclibc/lib/uclibc

include $(L4DIR)/mk/lib.mk
