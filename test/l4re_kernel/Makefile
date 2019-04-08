PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

TEST_GROUP := l4re-core/l4re_kernel

REQUIRES_LIBS  := libpthread atkins
DEPENDS_PKGS   := atkins

NED_CFG_test_nx = nx.cfg
SRC_S_test_nx := ARCH-$(ARCH)/nx.S

include $(L4DIR)/mk/test.mk
