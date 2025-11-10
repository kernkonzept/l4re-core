PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET      = libc_be_fs_noop.ofl libc_be_fs_noop.p.a
PC_FILENAME = libc_be_fs_noop
PC_LIBS       = %{-link-libc:%{shared:--whole-archive -lc_be_fs_noop.p --no-whole-archive;:libc_be_fs_noop.ofl}}
PC_LIBS_PIC   =
SRC_C       = fs.c

include $(L4DIR)/mk/lib.mk

CPPFLAGS-$(CONFIG_L4_LIBC_UCLIBC) = -DNOT_IN_libc -include libc-symbols.h
