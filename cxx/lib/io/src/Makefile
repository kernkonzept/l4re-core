PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		= libcxx_io
SRC_CC		= iob_write.cc
PC_FILENAME     = cxx_io
SYSTEMS		= $(SYSTEMS_PLAIN)

CXX_PKG_DIR=$(PKGDIR)
include $(PKGDIR)/lib/Makefile.inc
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(CXX_PKG_DIR)/lib/Makefile.inc
