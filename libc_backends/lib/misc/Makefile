PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET      = libc_support_misc.a libc_support_misc.so
PC_FILENAME = libc_support_misc
PC_EXTRA    = Link_Libs= %{static:-lc_support_misc}
SRC_C       = misc.c sched.c

include $(L4DIR)/mk/lib.mk
