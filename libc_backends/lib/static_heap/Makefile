PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET      = libc_be_static_heap.a
PC_FILENAME = libc_be_static_heap
PC_LIBS     =
PC_EXTRA    = Link_Libs= %{static|static-pie:-lc_be_static_heap}
SRC_CC      = heap.cc

include $(L4DIR)/mk/lib.mk
