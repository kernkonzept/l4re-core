PKGDIR = ../..
L4DIR ?= $(PKGDIR)/../..

TEST_GROUP := l4re-core/libc_be_socket_noop

# force static to avoid dependencies to stdlib-sh
# as we don't build anything in here at all
MODE := static

REQUIRES_LIBS := cxx_io cxx_libc_io libc_be_socket_noop
DEPENDS_PKGS  := atkins

include $(L4DIR)/mk/test.mk
