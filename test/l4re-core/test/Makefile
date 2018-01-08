PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

TEST_GROUP := l4re-core/l4re-core

REQUIRES_LIBS  := libstdc++ atkins
DEPENDS_PKGS   := atkins

PRIVATE_INCDIR := $(PKGDIR)/l4re-core/lib
PRIVATE_LIBDIR := $(PKGDIR_OBJ)/l4re-core/lib/OBJ-$(SYSTEM)

EXTRA_LIBS += -lthrowexc

include $(L4DIR)/mk/test.mk
