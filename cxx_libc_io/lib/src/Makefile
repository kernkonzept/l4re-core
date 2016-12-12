PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		= libcxx_libc_io
SRC_CC		= io_backend.cc
PC_FILENAME     = cxx_libc_io
PC_EXTRA        = Link_Libs= %{static:-lcxx_libc_io}

CXX_PKG_DIR=$(PKGDIR)/../cxx
include $(CXX_PKG_DIR)/lib/Makefile.inc
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(CXX_PKG_DIR)/lib/Makefile.inc
