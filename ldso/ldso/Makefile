PKGDIR ?= ..
L4DIR  ?= $(PKGDIR)/../../..

LIBCSRC_DIR ?= $(PKGDIR)/../libc/uclibc-ng

include $(LIBCSRC_DIR)/make_vars.mk

CON_DIR := $(CONTRIB_DIR)/ldso

PC_EXTRA = Link_DynLinker= rom/libld-l4.so.1

vpath %.c $(CON_DIR)/ldso
vpath %.S $(CON_DIR)/ldso/$(LIBC_ARCH)
vpath aeabi_read_tp-v6p.S     $(LIBCSRC_DIR)/libc/ARCH-arm
vpath aeabi_read_tp_generic.c $(LIBCSRC_DIR)/libc/ARCH-arm
vpath ssp-l4.c                $(LIBCSRC_DIR)/libc/ARCH-all/libc/misc/internals

PRIVATE_INCDIR-x86 := $(CONTRIB_DIR)/libc
PRIVATE_INCDIR  = $(PRIVATE_INCDIR-$(ARCH)) \
                  $(OBJ_BASE)/include/uclibc-ng \
                  $(CON_DIR)/ldso/$(LIBC_ARCH) \
                  $(SRC_DIR) \
                  $(CON_DIR)/include \
                  $(PTHREAD_INCDIR)


TARGET := libld-l4.so
SRC_C  := ldso.c dl-vdso.c fixup.c string.c
SRC_CC := syscalls.cc vfs.cc
#SRC_S_arm-l4f  += aeabi_read_tp-v6p.S
SRC_C_arm-l4f   += aeabi_read_tp_generic.c
SRC_S  := resolve.S

CFLAGS += -ffreestanding
CFLAGS_string.c = -fvisibility=hidden
CFLAGS_fixup.c = -fvisibility=hidden

CPPFLAGS := -DNOT_IN_libc -DIS_IN_rtld \
            -DLDSO_ELFINTERP=\"$(LIBC_ARCH)/elfinterp.c\" \
            -include libc-symbols.h -DUCLIBC_LDSO=\"libld-l4.so.1\" \
            -DUCLIBC_RUNTIME_PREFIX=\"/\" \
            -D__LDSO_SEARCH_INTERP_PATH__=1 \
            -DIS_IN_rtld -D__LIBDL_SHARED__ -DSHARED

DEFINES += -DUSE_TLS=1

LDFLAGS  =  -nocrt1 -Bsymbolic -Bstatic --warn-common --sort-common \
            --no-undefined -e _start -z defs -z now \
            --discard-locals --discard-all   \
            --gc-sections --whole-archive -ll4sys-direct --no-whole-archive \
            -static-libgcc

SOVERSION = 1

include $(L4DIR)/mk/Makeconf

# stack protector handling
ifdef CONFIG_BID_GCC_ENABLE_STACK_PROTECTOR
DEFINES  += -D__UCLIBC_HAS_SSP__=1
SRC_C    += ssp-l4.c
endif

include $(L4DIR)/mk/lib.mk

ldso.1.o: ldso.o
	$(OBJCOPY) --weaken $< $@

CXXFLAGS := $(filter-out -fno-omit-frame-pointer, $(CXXFLAGS))
CXXFLAGS := $(filter-out -fno-strict-aliasing, $(CXXFLAGS))
CXXFLAGS += -DL4_NO_RTTI -fno-rtti -fno-exceptions -fvisibility=hidden \
            -fno-threadsafe-statics -O3

CPPFLAGS := $(filter-out -DL4SYS_USE_UTCB_WRAP=1, $(CPPFLAGS))

# Suppress false-positive warnings
WARNINGS += -Wno-bad-function-cast
