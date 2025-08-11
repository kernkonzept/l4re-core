PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../..

REQUIRES_LIBS	= cxx_util

TARGET		= libumalloc.a
PC_EXTRA	= Link_Libs= %{static|static-pie:-lumalloc}

SRC_CC		= malloc.cc

include $(L4DIR)/mk/lib.mk
