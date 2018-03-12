PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../../..

PC_FILENAME     = libc_be_sig
TARGET		= libc_be_sig.a libc_be_sig.so
SRC_CC          = sig.cc
REQUIRES_LIBS   = l4re-util libpthread
PRIVATE_INCDIR  = $(SRC_DIR)/ARCH-$(ARCH)
CXXFLAGS        = -fno-exceptions

include $(L4DIR)/mk/lib.mk
