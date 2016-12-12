PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../..

TARGET        = libm_support.a libm_support.so
SRC_C         = sincos.c log2f.c exp2f.c _sin.c complex.c
PC_FILENAME   = libc_be_math
PC_EXTRA      = Link_libs= %{static:-lm_support}
REQUIRES_LIBS = libc

include $(L4DIR)/mk/lib.mk
