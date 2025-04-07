PKGDIR		?= ..
L4DIR		?= $(PKGDIR)/../../..

SYSTEMS		?= $(SYSTEMS_PLAIN)

include $(L4DIR)/mk/Makeconf

LIBCSRC_DIR	?= $(PKGDIR)/../libc/$(CONFIG_L4_LIBC)

include $(LIBCSRC_DIR)/src_rules.mk

