PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		= libcxx_atexit
SYSTEMS		= $(SYSTEMS_PLAIN)
SRC_CC		= cxx_atexit.cc
PC_FILENAME     = cxx_atexit

CXX_PKG_DIR=$(PKGDIR)
include $(PKGDIR)/lib/Makefile.inc
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(CXX_PKG_DIR)/lib/Makefile.inc
