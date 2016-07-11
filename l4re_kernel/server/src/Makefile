PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET          = l4re
SRC_CC         := main.cc globals.cc region.cc loader.cc \
                  loader_elf.cc mem.cc dispatcher.cc

SRC_S_mips     := loader_mips.S
CXXFLAGS_mips += -mplt -mno-shared -DL4_LOADER_USE_ASM_STUB=1
#CXXFLAGS_arm64 += -mcmodel=large

DEFAULT_RELOC_x86   := 0xb0000000
DEFAULT_RELOC_arm   := 0xb0000000
DEFAULT_RELOC_arm64 := 0xc0000000
DEFAULT_RELOC_ppc32 := 0xb0000000
DEFAULT_RELOC_amd64 := 0x70000000
DEFAULT_RELOC_mips  := 0x70000000

MODE = sigma0

PRIVATE_INCDIR      += $(SRC_DIR)/ARCH-$(ARCH)

DEFINES += -DL4_CXX_NO_EXCEPTION_BACKTRACE

REQUIRES_LIBS  := libc_be_minimal_log_io cxx_io cxx_libc_io libc_minimal l4re \
                  l4re-util libsupc++_minimal libloader libc_minimal_l4re

CXXFLAGS += $(CXXFLAGS_LOW_LEVEL)

CXXFLAGS += $(CXXFLAGS_$(ARCH))
SRC_S += $(SRC_S_$(ARCH))

vpath %.S $(SRC_DIR)/ARCH-$(ARCH)

include $(L4DIR)/mk/prog.mk

