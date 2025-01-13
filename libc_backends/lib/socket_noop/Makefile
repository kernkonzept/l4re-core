PKGDIR     ?= ../..
L4DIR      ?= $(PKGDIR)/../../..

TARGET      = libc_be_socket_noop.ofl libc_be_socket_noop.p.a
PC_FILENAME = libc_be_socket_noop
PC_LIBS     = %{-link-libc:%{shared:--whole-archive -lc_be_socket_noop.p --no-whole-archive;:libc_be_socket_noop.ofl}}
PC_LIBS_PIC =
SRC_C       = socket_noop.c

include $(L4DIR)/mk/lib.mk
