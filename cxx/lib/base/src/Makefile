PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		= libcxx_base
SYSTEMS		= $(SYSTEMS_PLAIN)
PC_FILENAME     = cxx_base
SRC_CC		= cxa_pure_virtual.cc cxa_pure_delete.cc
SRC_CC_arm     	= dso_handle.cc

CXX_PKG_DIR=$(PKGDIR)
include $(PKGDIR)/lib/Makefile.inc
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(CXX_PKG_DIR)/lib/Makefile.inc
