PKGDIR	?= .
L4DIR	?= $(PKGDIR)/../../..
STDCXX_PKG_DIR ?= $(PKGDIR_ABS)/../libstdc++-v3

include $(L4DIR)/mk/Makeconf
include $(STDCXX_PKG_DIR)/contrib.inc

TARGET := include include-$(STDCXX_CONTRIB_VERSION) include-std

include $(L4DIR)/mk/subdir.mk

