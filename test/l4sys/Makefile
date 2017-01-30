PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../..

TEST_GROUP := l4re-core/l4sys

REQUIRES_LIBS  := libpthread atkins
DEPENDS_PKGS   := atkins

EXTRA_TEST := test_sched_proxy

MOE_CFG_test_sched_proxy := test_scheduler.cfg

TEST_TARGET_test_sched_proxy := test_scheduler

include $(L4DIR)/mk/test.mk
