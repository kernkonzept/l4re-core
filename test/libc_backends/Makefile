PKGDIR = ../..
L4DIR ?= $(PKGDIR)/../..

TEST_GROUP    := l4re-core/libc_backends

NED_CFG_test_l4re_file = l4re_file.cfg
REQUIRES_LIBS := atkins
REQUIRES_LIBS += libc_be_fs_noop libc_be_sem_noop libc_be_sig_noop
DEPENDS_PKGS  := atkins

REQUIRED_MODULES_test_l4re_file += l4re_file_testfile_ro
REQUIRED_MODULES_test_l4re_file += l4re_file_testfile_rw:rw

include $(L4DIR)/mk/test.mk
