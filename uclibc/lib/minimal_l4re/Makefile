# vi:ft=make
PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../../..

TARGET          := libuc_c_minimal_l4re.a
PC_FILENAME     := libc_minimal_l4re
define PC_EXTRA
Link_Libs= %{static:-luc_c_minimal_l4re}
endef

DEFINES         := -DL4_MINIMAL_LIBC
SRC_CC          := _exit.cc

vpath _exit.cc $(SRC_DIR)/../uclibc

include $(L4DIR)/mk/lib.mk
