PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET      = libc_be_fs_noop.a libc_be_fs_noop.so
PC_FILENAME = libc_be_fs_noop
PC_EXTRA    = Link_Libs= %{static:-lc_be_fs_noop}
SRC_C       = tbd.c

include $(L4DIR)/mk/lib.mk
