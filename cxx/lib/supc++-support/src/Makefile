PKGDIR		?= ../../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		= libsupc++-support
SRC_C		= memset.c 
SRC_CC		= abort.cc

CXX_PKG_DIR=$(PKGDIR)
include $(PKGDIR)/lib/Makefile.inc
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(CXX_PKG_DIR)/lib/Makefile.inc
