PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET      = libc_be_fs_noop.a libc_be_fs_noop.so
PC_FILENAME = libc_be_fs_noop
PC_EXTRA    = Link_Libs= %{static|static-pie:-lc_be_fs_noop}
SRC_C       = fs.c
CFLAGS      = -DNOT_IN_libc -include libc-symbols.h

include $(L4DIR)/mk/lib.mk
