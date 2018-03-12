PKGDIR ?= 	..
L4DIR  ?= 	$(PKGDIR)/../../..

TARGET         ?= libsupc++.a libsupc++.so
PC_FILENAME    ?= libsupc++
PC_LIBS        ?= -lsupc++
REQUIRES_LIBS  ?= libc

CONTRIB_INCDIR  = libstdc++-v3

STDCXX_PKG_DIR ?= $(PKGDIR)/../libstdc++-v3

STDCXX_SRC_DIR ?= $(STDCXX_CONTRIB_DIR)/libsupc++
VPATH_SRC_BASE  = $(STDCXX_SRC_DIR)
PRIVATE_INCDIR  = $(STDCXX_PKG_DIR)/contrib/gcc-$(STDCXX_CONTRIB_VERSION) \
		  $(STDCXX_SRC_DIR)

SRC_CC := \
del_op.cc \
del_opnt.cc \
del_opv.cc \
del_opvnt.cc \
eh_alloc.cc \
eh_arm.cc \
eh_aux_runtime.cc \
eh_call.cc \
eh_catch.cc \
eh_exception.cc \
eh_globals.cc \
eh_personality.cc \
eh_term_handler.cc \
eh_terminate.cc \
eh_throw.cc \
eh_type.cc \
eh_unex_handler.cc \
guard.cc \
new_handler.cc \
new_op.cc \
new_opnt.cc \
new_opv.cc \
new_opvnt.cc \
pure.cc \
tinfo2.cc \
tinfo.cc \
vec.cc \
vterminate.cc \
demangle.cc

SUPCXX_PKG_DIR ?= $(PKGDIR)

include $(STDCXX_PKG_DIR)/contrib.inc
include $(SUPCXX_PKG_DIR)/build/Makefile-$(STDCXX_CONTRIB_VERSION)
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(SUPCXX_PKG_DIR)/build/Makefile-$(STDCXX_CONTRIB_VERSION) $(STDCXX_PKG_DIR)/contrib.inc

vpath %.cc $(STDCXX_SRC_DIR)
vpath %.cc $(SRC_DIR)
