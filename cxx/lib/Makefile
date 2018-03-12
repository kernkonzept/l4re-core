PKGDIR	= ..
L4DIR	?= $(PKGDIR)/../../..
TARGET	= io base be start util tl ipc

include $(L4DIR)/mk/subdir.mk

base: io
be: io
start: base util
util: io
ipc: base tl

