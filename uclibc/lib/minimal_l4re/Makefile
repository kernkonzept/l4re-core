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

SRC_S_arm64-l4f  = exception-el1.S
SRC_CC_arm64-l4f = exception-el1-handler.cc

vpath exception-el1.S $(SRC_DIR)/../uclibc/ARCH-arm64
vpath exception-el1-handler.cc $(SRC_DIR)/../uclibc/ARCH-arm64
vpath _exit.cc $(SRC_DIR)/../uclibc

include $(L4DIR)/mk/lib.mk
