include $(L4DIR)/mk/Makeconf

LIBCSRC_DIR ?= $(SRC_DIR)
LIBCSRC_DIR_ABS := $(abspath $(LIBCSRC_DIR))

# directory to the libc contrib files
CONTRIB_DIR     := $(LIBCSRC_DIR_ABS)/../contrib/uclibc
CONTRIB_SYSDEPS := $(CONTRIB_DIR)/libc/sysdeps/linux

# destination directory for linking libc sources, with our overlay
LIBC_DST_DIR  := $(OBJ_DIR)/src

# build prefix for pthread sources
PTHOBJ_PFX    := libpthread/src

# pthread source file directory
PTHLIB_DIR    := $(LIBCSRC_DIR_ABS)/../libpthread

# include directory for pthread internals
PTHREAD_INCDIR = $(PTHLIB_DIR)/src/sysdeps/$(LIBC_ARCH) $(PTHLIB_DIR)/src

# include dir for accessing ldso internals
LDSO_INC = -I$(CONTRIB_DIR)/ldso/ldso/$(LIBC_ARCH) -I$(CONTRIB_DIR)/ldso/include


LIBC_ARCH_x86   := i386
LIBC_ARCH_arm   := arm
LIBC_ARCH_arm64 := aarch64
LIBC_ARCH_amd64 := x86_64
LIBC_ARCH_mips  := mips
LIBC_ARCH_ppc32 := powerpc
LIBC_ARCH_riscv := riscv
LIBC_ARCH_sparc := sparc

LIBC_ARCH       := $(LIBC_ARCH_$(BUILD_ARCH))

-include $(DEPSVAR)
