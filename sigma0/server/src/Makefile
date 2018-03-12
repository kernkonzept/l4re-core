PKGDIR		    ?= ../..
L4DIR		    ?= $(PKGDIR)/../../..

TARGET		     = sigma0
MODE		     = sigma0
L4_LIBS		     = -static -nostdlib $(GCCLIB)
REQUIRES_LIBS        = libc_minimal cxx_util cxx_io cxx_base \
                       cxx_io_kdebug l4sys-direct l4sys
DEFINES             += -DL4_MINIMAL_LIBC
DEFAULT_RELOC_x86   := 0x00100000
DEFAULT_RELOC_amd64 := 0x00100000
DEFAULT_RELOC_arm   := 0x000c0000
DEFAULT_RELOC_mips  := 0x00200000
DEFAULT_RELOC_sparc := 0x01200000
DEFAULT_RELOC_ppc32 := 0x00100000
RELOC_PHYS          := y

PRIVATE_INCDIR	= $(SRC_DIR)

SRC_CC := init.cc memmap.cc page_alloc.cc mem_man.cc io_backend.cc \
          init_mem.cc
SRC_CC_x86-l4f   := ioports_x86.cc
SRC_CC_amd64-l4f := ioports_x86.cc
SRC_CC_arm-l4f   := ioports_none.cc
SRC_CC_mips-l4f  := ioports_none.cc
SRC_CC_ppc32-l4f := ioports_none.cc
SRC_CC_sparc-l4f := ioports_none.cc
SRC_C  := support.c
SRC_S  := $(addprefix ARCH-$(ARCH)/, crt0.S)

include $(L4DIR)/mk/prog.mk
CRT1_stat_bin   =

CXXFLAGS	+= -DL4_NO_RTTI -fno-rtti -fno-exceptions -Wall
