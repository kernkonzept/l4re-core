PKGDIR = ../..
L4DIR ?= $(PKGDIR)/../..

TEST_GROUP    := l4re-core/libc_be_minimal_io

# force sigma0 mode to avoid pulling in all of stdlib-sh
MODE := sigma0

REQUIRES_LIBS := cxx_io cxx_libc_io libc_be_minimal_log_io libc cxx_base
DEPENDS_PKGS  := atkins

include $(L4DIR)/mk/test.mk
