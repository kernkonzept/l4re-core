# vim:se ft=make:
include $(L4DIR)/mk/Makeconf

LIBGCC_CONTRIB_MIN_VERSION := 9
LIBGCC_CONTRIB_MAX_VERSION := 13
LIBGCC_CONTRIB_SUPPORTED_VERSIONS := $(shell seq $(LIBGCC_CONTRIB_MIN_VERSION) $(LIBGCC_CONTRIB_MAX_VERSION))
LIBGCC_CONTRIB_VERSION := $(GCCVERSION)

ifeq ($(filter $(LIBGCC_CONTRIB_VERSION),$(LIBGCC_CONTRIB_SUPPORTED_VERSIONS)),)
$(warning No suitable libgcc version found for gcc $(LIBGCC_CONTRIB_VERSION).)
$(warning Falling back to libgcc $(LIBGCC_CONTRIB_MAX_VERSION), unexpected errors may occur.)
$(warning Please update the libgcc package.)
$(warning In case of errors, as a temporary workaround the libgcc shipped with the toolchain \
          can be used by enabling the Kconfig option COMPILER_RT_USE_TOOLCHAIN_LIBGCC.)
LIBGCC_CONTRIB_VERSION := $(LIBGCC_CONTRIB_MAX_VERSION)
endif

LIBGCC_CONTRIB_VERSIONS := $(shell seq $(LIBGCC_CONTRIB_VERSION) -1 $(LIBGCC_CONTRIB_MIN_VERSION))
LIBGCC_CONTRIB_SUFFIX_VERSIONS := $(addprefix -,$(LIBGCC_CONTRIB_VERSIONS))

LIBGCC_CONTRIB_DIR = $(LIBGCC_PKG_DIR)/contrib/gcc-$(LIBGCC_CONTRIB_VERSION)
LIBGCC_GENERATED_DIR = $(LIBGCC_PKG_DIR)/generated
LIBGCC_CONTRIB_SRC_DIR = $(LIBGCC_CONTRIB_DIR)/libgcc

CPU_TYPE_x86   := i386
CPU_TYPE_amd64 := i386
CPU_TYPE_arm   := arm
CPU_TYPE_arm64 := aarch64
CPU_TYPE_mips  := mips
CPU_TYPE_riscv := riscv
CPU_TYPE        = $(CPU_TYPE_$(BUILD_ARCH))
