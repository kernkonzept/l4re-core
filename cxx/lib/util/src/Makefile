PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		= libcxx_util
PC_FILENAME     = cxx_util
SRC_CC		= ipc_error_str.cc alloc_list.cc
SYSTEMS		= $(SYSTEMS_PLAIN)

CXX_PKG_DIR=$(PKGDIR)
include $(PKGDIR)/lib/Makefile.inc
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(CXX_PKG_DIR)/lib/Makefile.inc
