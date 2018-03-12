PKGDIR	= .
L4DIR	?= $(PKGDIR)/../../..
TARGET	= lib

include $(L4DIR)/mk/subdir.mk

include:
	$(foreach DIR, $(SUBDIR), $(MAKE) -C $(DIR) include)
