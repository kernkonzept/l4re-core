PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET          = l4re
SRC_CC         := main.cc globals.cc region.cc loader.cc \
                  loader_elf.cc dispatcher.cc signals.cc remote_access.cc
SRC_CC         += $(if $(CONFIG_BID_STATIC_HEAP),mem_stat.cc,mem_dyn.cc)

SRC_S          := ARCH-$(ARCH)/sig_trampoline.S

SRC_S_mips     := ARCH-mips/loader_mips.S
SRC_S_riscv    := ARCH-riscv/loader_riscv.S
CXXFLAGS_mips  += -mplt -mno-shared -DL4_LOADER_USE_ASM_STUB=1
CXXFLAGS_riscv += -DL4_LOADER_USE_ASM_STUB=1
#CXXFLAGS_arm64 += -mcmodel=large

DEFAULT_RELOC_x86   := 0xb0000000
DEFAULT_RELOC_arm   := 0xb0000000
DEFAULT_RELOC_arm64 := 0xc0000000
DEFAULT_RELOC_ppc32 := 0xb0000000
DEFAULT_RELOC_amd64 := 0x70000000
DEFAULT_RELOC_mips  := 0x70000000
DEFAULT_RELOC_riscv := 0x70000000

MODE = sigma0

PRIVATE_INCDIR      += $(SRC_DIR)/ARCH-$(ARCH)

DEFINES += -DL4_CXX_NO_EXCEPTION_BACKTRACE -DL4_LOADER_RELOC_BASE=$(DEFAULT_RELOC_$(ARCH))

REQUIRES_LIBS  := cxx_io cxx_libc_io libc_minimal libsupc++_minimal libloader \
                  libc_minimal_l4re libumalloc

CXXFLAGS += $(CXXFLAGS_LOW_LEVEL)

include $(L4DIR)/mk/prog.mk
