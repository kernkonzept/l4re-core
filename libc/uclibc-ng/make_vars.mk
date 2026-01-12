include $(L4DIR)/mk/Makeconf

LIBCSRC_DIR ?= $(SRC_DIR)
LIBCSRC_DIR_ABS := $(abspath $(LIBCSRC_DIR))

# directory to the libc contrib files
CONTRIB_DIR     := $(LIBCSRC_DIR_ABS)/contrib/uclibc
CONTRIB_SYSDEPS := $(CONTRIB_DIR)/libc/sysdeps/linux

# destination directory for linking libc sources, with our overlay
LIBC_DST_DIR  := $(OBJ_DIR)/src

# source directories for finding contrib source files (and overrides)
LIBC_SRC_DIRS := $(CONTRIB_DIR)/libc \
                 $(LIBCSRC_DIR_ABS)/libc/ARCH-all/libc

LIBC_SRC_DIRS += $(CONTRIB_DIR)/libm \
                 $(CONTRIB_DIR)/libcrypt \
                 $(CONTRIB_DIR)/libiconv \
                 $(CONTRIB_DIR)/libuargp

# pthread source file directory
PTHLIB_DIR    := $(PKGDIR)/../libpthread

# include directory for pthread internals
PTHREAD_INCDIR  = $(PTHLIB_DIR)/src/sysdeps/$(LIBC_ARCH) $(PTHLIB_DIR)/src
PTHREAD_INCDIR += $(LIBCSRC_DIR)/libc/includes

# include dir for accessing ldso internals
LDSO_INC = -I$(CONTRIB_DIR)/ldso/ldso/$(LIBC_ARCH) -I$(CONTRIB_DIR)/ldso/include

LIBC_DYNLINKER = /rom/libld-l4.so.1


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
