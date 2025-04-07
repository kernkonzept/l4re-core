# directories we need to know
PKGDIR  = .
L4DIR  ?= $(PKGDIR)/../../..

TARGET = $(wildcard $(CONFIG_L4_LIBC))

# include subdir role
include $(L4DIR)/mk/subdir.mk

ifeq ($(TARGET),)
  $(error The configured c library: $(CONFIG_L4_LIBC) was not found)
endif

