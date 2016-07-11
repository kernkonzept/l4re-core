include $(L4DIR)/mk/Makeconf

LIBCSRC_DIR ?= $(SRC_DIR)
LIBCSRC_DIR_ABS := $(call absfilename,$(LIBCSRC_DIR))

SYSDEPS          := libc/sysdeps/linux

# directory to the libc contrib files
CONTRIB_DIR   := $(LIBCSRC_DIR_ABS)/../contrib/uclibc

# destination directory for linking libc sources, with our overlay
LIBC_DST_DIR  := $(OBJ_DIR)/src

# build prefix for pthread sources
PTHOBJ_PFX    := libpthread/src

# pthread source file directory
PTHLIB_DIR    := $(LIBCSRC_DIR_ABS)/../libpthread

# include directory for pthread internals
PTHREAD_INCDIR = $(PTHLIB_DIR)/src/sysdeps/$(UCLIBC_ARCH_$(ARCH)) \
                 $(PTHLIB_DIR)/src

# include dir for accessing ldso internals
LDSO_INC = -I$(CONTRIB_DIR)/ldso/ldso/$(UCLIBC_ARCH) -I$(CONTRIB_DIR)/ldso/include


UCLIBC_ARCH_x86   := i386
UCLIBC_ARCH_arm   := arm
UCLIBC_ARCH_arm64 := arm64
UCLIBC_ARCH_amd64 := x86_64
UCLIBC_ARCH_mips  := mips
UCLIBC_ARCH_ppc32 := powerpc
UCLIBC_ARCH_sparc := sparc

UCLIBC_ARCH       := $(UCLIBC_ARCH_$(BUILD_ARCH))

-include $(DEPSVAR)
