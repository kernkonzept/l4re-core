PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET         = libc_be_l4refile.ofl libc_be_l4refile.p.a
REQUIRES_LIBS  = l4re
PC_FILENAME    = libc_be_l4refile
PC_LIBS        = %{-link-libc:%{shared:--whole-archive -lc_be_l4refile.p --no-whole-archive;:libc_be_l4refile.ofl}}
PC_LIBS_PIC    =
SRC_CC         = file.cc mmap.cc mount.cc socket.cc
# No exception information as unwinder code might uses malloc and friends
CXXFLAGS       := -DL4_NO_RTTI -fno-rtti -fno-exceptions \
                  -DNOT_IN_libc -include libc-symbols.h

include $(L4DIR)/mk/lib.mk
