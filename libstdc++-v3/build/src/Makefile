PKGDIR ?= 	../..
L4DIR  ?= 	$(PKGDIR)/../../..

TARGET         ?= libstdc++.a libstdc++.so
PC_FILENAME    ?= libstdc++
PC_LIBS        ?= -lstdc++
REQUIRES_LIBS  ?= libsupc++
STDCXX_PKG_DIR ?= $(PKGDIR)

PRIVATE_INCDIR += $(STDCXX_PKG_DIR)/private_inc

include $(STDCXX_PKG_DIR)/contrib.inc
include $(STDCXX_PKG_DIR)/build/src/Makefile-$(STDCXX_CONTRIB_VERSION)
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(STDCXX_PKG_DIR)/build/src/Makefile-$(STDCXX_CONTRIB_VERSION)

vpath %.cc $(OBJ_DIR)
vpath %.cc $(STDCXX_CONTRIB_DIR)/config/io
vpath %.cc $(STDCXX_CONTRIB_DIR)/config/locale/generic

vpath %.cc $(STDCXX_CONTRIB_DIR)/src
