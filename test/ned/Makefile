PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

MODE = static

TEST_GROUP := l4re-core/ned

REQUIRES_LIBS  := libpthread atkins
DEPENDS_PKGS   := atkins

MOE_CFG_test_exit_wait := exit_wait.cfg

include $(L4DIR)/mk/test.mk
