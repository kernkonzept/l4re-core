PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET         = libc_be_l4refile.a libc_be_l4refile.so
REQUIRES_LIBS  = l4re libsupc++
PC_FILENAME    = libc_be_l4refile
PC_LIBS        = -lc_be_l4refile
PC_EXTRA       = Link_Libs= %{static:-lc_be_l4refile}
SRC_CC         = file.cc mmap.cc mount.cc socket.cc
# No exception information as unwinder code might uses malloc and friends
CXXFLAGS       := -fno-exceptions

include $(L4DIR)/mk/lib.mk
