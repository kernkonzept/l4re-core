PKGDIR	?= .
L4DIR	?= $(PKGDIR)/../../..

TARGET = lib server ned-prompt

include $(L4DIR)/mk/subdir.mk

# ned-prompt and server depend on the includes from lib/include
ned-prompt server: lib
