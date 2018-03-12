PKGDIR ?= .
L4DIR  ?= $(PKGDIR)/../../..

STDCXX_PKG_DIR =  $(PKGDIR)/../libstdc++-v3
SUPCXX_PKG_DIR =  $(PKGDIR)/../libsupc++

TARGET          = libsupc++_minimal.a
PC_FILENAME     = libsupc++_minimal
PC_LIBS		= -lsupc++_minimal
REQUIRES_LIBS   = libc_minimal
DEFINES        += -DL4_MINIMAL_LIBC

include $(PKGDIR)/../libsupc++/build/Makefile

vpath %.cc $(SUPCXX_PKG_DIR)/build
