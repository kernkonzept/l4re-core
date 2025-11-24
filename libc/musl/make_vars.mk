include $(L4DIR)/mk/Makeconf

LIBCSRC_DIR ?= $(SRC_DIR)
LIBCSRC_DIR_ABS := $(abspath $(LIBCSRC_DIR))

# SYSDEPS := ?

CONTRIB_DIR := $(LIBCSRC_DIR_ABS)/contrib/musl

LIBC_SRC_DIRS := $(CONTRIB_DIR)/ldso $(CONTRIB_DIR)/src $(LIBCSRC_DIR_ABS)/libc/ARCH-all/src
LIBC_DST_DIR  := $(OBJ_DIR)/src

# build prefix for pthread sources
PTHOBJ_PFX    := libpthread/src

# pthread source file directory
PTHLIB_DIR    := $(LIBCSRC_DIR_ABS)/libpthread

# include directory for pthread internals
PTHREAD_INCDIR = $(PTHLIB_DIR)/src/sysdeps/$(LIBC_ARCH) $(PTHLIB_DIR)/src

LIBC_DYNLINKER = /rom/libc.so.1

LIBC_ARCH_x86 := i386
LIBC_ARCH_arm := arm
LIBC_ARCH_arm64 := aarch64
LIBC_ARCH_amd64 := x86_64
LIBC_ARCH_mips := mips
LIBC_ARCH_ppc32 := powerpc
LIBC_ARCH_riscv := riscv$(BITS)

# SPARC is dead ...
ifeq ($(BUILD_ARCH),sparc)
  $(error Musl does not support sparc)
endif

LIBC_ARCH := $(LIBC_ARCH_$(BUILD_ARCH))

-include $(DEPSVAR)
