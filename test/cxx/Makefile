PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

TEST_GROUP    := l4re-core/cxx

SRC_CC_test_item += item.cc
SRC_CC_test_avl_set += item.cc

REQUIRES_LIBS := libstdc++ atkins
DEPENDS_PKGS  := atkins

include $(L4DIR)/mk/test.mk
