PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET	 = libkproxy.so libkproxy.a
SRC_CC	 = factory_svr.cc

# Hm, warum is das hier drin?
CXXFLAGS += -DL4_NO_RTTI -fno-rtti

include $(L4DIR)/mk/lib.mk
