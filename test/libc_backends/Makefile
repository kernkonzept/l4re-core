PKGDIR = ../..
L4DIR ?= $(PKGDIR)/../..

TEST_GROUP    := l4re-core/libc_backends

REQUIRES_LIBS := atkins
REQUIRES_LIBS += libc_be_fs_noop libc_be_sem_noop libc_be_sig_noop
DEPENDS_PKGS  := atkins

include $(L4DIR)/mk/test.mk
