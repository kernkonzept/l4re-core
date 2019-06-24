PKGDIR ?= ../../..
L4DIR  ?= $(PKGDIR)/../..

TEST_GROUP := l4re-core/uclibc/pthread_netbsd

TEST_EXPECTED_test_atexit = test_atexit.output

REQUIRES_LIBS  := libpthread libstdc++ libgtest atkins
DEPENDS_PKGS   := atkins

include $(L4DIR)/mk/test.mk
