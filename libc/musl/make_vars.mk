include $(L4DIR)/mk/Makeconf

LIBCSRC_DIR ?= $(SRC_DIR)
LIBCSRC_DIR_ABS := $(abspath $(LIBCSRC_DIR))

# SYSDEPS := ?

CONTRIB_DIR := $(LIBCSRC_DIR_ABS)/contrib/musl

LIBC_SRC_DIRS := $(CONTRIB_DIR)/ldso $(CONTRIB_DIR)/src $(LIBCSRC_DIR_ABS)/libc/ARCH-all/src
LIBC_DST_DIR  := $(OBJ_DIR)/src

# pthread source file directory
PTHLIB_DIR    := $(PKGDIR)/../libpthread

# include directory for pthread internals
PTHREAD_INCDIR  = $(PTHLIB_DIR)/src/sysdeps/$(LIBC_ARCH_FAMILY) $(PTHLIB_DIR)/src
PRIVATE_INCDIR += $(LIBCSRC_DIR)/libc/includes

LIBC_DYNLINKER = /rom/libc.so.1

# Warnings that must be disabled when compiling musl contrib sources. They are
# either idiomatic in musl (K&R style declarations of generic function
# pointers) or cannot be fixed in the sources at all (GCC does not implement
# "#pragma STDC FENV_ACCESS"). Used by the libc build itself and by every other
# package that compiles musl sources (e.g. ldscripts for the crt files).
LIBC_MUSL_WARNINGS = -Wno-strict-prototypes \
                     -Wno-missing-prototypes \
                     -Wno-missing-declarations \
                     -Wno-unused-parameter \
                     -Wno-unused-function \
                     -Wno-parentheses \
                     -Wno-unused-but-set-variable \
                     -Wno-sign-compare \
                     -Wno-unknown-pragmas

# LIBC_ARCH selects the (ABI-specific) musl contrib arch directory
# (contrib/musl/arch/<LIBC_ARCH> and contrib/musl/src/*/<LIBC_ARCH>).
LIBC_ARCH_x86 := i386
LIBC_ARCH_arm := arm
LIBC_ARCH_arm64 := aarch64
LIBC_ARCH_amd64 := x86_64
# MIPS has one musl arch dir per ABI: o32 -> mips, n32 -> mipsn32, n64 -> mips64.
LIBC_ARCH_mips_32 := mips
LIBC_ARCH_mips_n32 := mipsn32
LIBC_ARCH_mips_64 := mips64
LIBC_ARCH_mips := $(LIBC_ARCH_mips_$(CPU_ABI))
LIBC_ARCH_riscv := riscv$(BITS)

LIBC_ARCH := $(LIBC_ARCH_$(BUILD_ARCH))

# LIBC_ARCH_FAMILY is the ABI-agnostic CPU family name used for the L4Re-provided
# glue that is written to cover all ABIs of a CPU family via the preprocessor
# (libc/ARCH-<family> and libpthread sysdeps/<family>). It matches LIBC_ARCH for
# every arch except MIPS and RISC-V, whose ABI-specific LIBC_ARCH (e.g. mips64,
# riscv64) differs from the shared family directory name (mips, riscv).
LIBC_ARCH_FAMILY_mips := mips
LIBC_ARCH_FAMILY_riscv := riscv
LIBC_ARCH_FAMILY := $(or $(LIBC_ARCH_FAMILY_$(BUILD_ARCH)),$(LIBC_ARCH))

-include $(DEPSVAR)
