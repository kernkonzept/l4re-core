PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET         = libc_be_l4refile.a libc_be_l4refile.so
REQUIRES_LIBS  = l4re
PC_FILENAME    = libc_be_l4refile
PC_LIBS        = -lc_be_l4refile
PC_EXTRA       = Link_Libs= %{static|static-pie:-lc_be_l4refile}
SRC_CC         = file.cc mmap.cc mount.cc socket.cc
# No exception information as unwinder code might uses malloc and friends
CXXFLAGS       := -DL4_NO_RTTI -fno-rtti -fno-exceptions \
                  -DNOT_IN_libc -include libc-symbols.h

include $(L4DIR)/mk/lib.mk
