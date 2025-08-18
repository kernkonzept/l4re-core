# vi:ft=make

include $(L4DIR)/mk/Makeconf

ifeq ($(intcmp 0,0),)
# No intcmp supported (pre Make 4.4)
cmp = $(shell perl -e 'print [@ARGV[2..4]]->[1+($$ARGV[0]<=>$$ARGV[1])]' "$1" "$2" "$3" "$4" "$5")
else
# intcmp supported
cmp = $(intcmp $1,$2,$3,$4,$5)
endif

# Must be numerically sorted
STDCXX_AVAILABLE_VERSIONS = 9 10 11 12 13 14 15 16
_max = $(lastword $(STDCXX_AVAILABLE_VERSIONS))
_min = $(firstword $(STDCXX_AVAILABLE_VERSIONS))

ifeq ("$(BID_COMPILER_TYPE)", "clang")
  STDCXX_CONTRIB_VERSION := 12
else
  # Choose STDCXX_CONTRIB_VERSION = GCCVERSION if available
  # Else choose lowest available if lower, choose highest available if higher
  STDCXX_CONTRIB_VERSION := $(or $(filter $(GCCVERSION),$(STDCXX_AVAILABLE_VERSIONS)),\
                            $(call cmp,$(GCCVERSION),$(_max),$(_min),$(_max),$(_max)))
endif
# Just check if chosen version is 10 or higher
STDCXX_CONTRIB_VERSION_10_PLUS  := $(call cmp, $(STDCXX_CONTRIB_VERSION),10,,y,y)

STDCXX_CONTRIB_DIR = $(STDCXX_PKG_DIR)/contrib/libstdc++-v3-$(STDCXX_CONTRIB_VERSION)
DEFINES += -Dlinux

CPU_H_x86   := i386
CPU_H_ppc32 := powerpc
CPU_H_arm   := arm
CPU_H_amd64 := i386
CPU_H_mips  := mips
CPU_H_sparc := sparc

CPU_H_generic := config/cpu/generic
CPU_H_arch     = config/cpu/$(CPU_H_$(BUILD_ARCH))

CPU_file = $(word 1, $(wildcard $(STDCXX_CONTRIB_DIR)/$(CPU_H_arch)/$(strip $(1)) $(STDCXX_CONTRIB_DIR)/$(CPU_H_generic)/$(strip $(1))))

ifeq ($(BID_COMPILER_TYPE),clang)
  CCXX_FLAGS += -fsized-deallocation
  DEFINES += -DL4_CLANG_FIXES
endif

#$(info GCC: $(STDCXX_CONTRIB_DIR))

