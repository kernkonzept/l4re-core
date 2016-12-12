PKGDIR ?= ..
L4DIR  ?= $(PKGDIR)/../../..

CON_DIR := $(PKGDIR)/../uclibc/lib/contrib/uclibc/ldso
PTH_DIR := $(PKGDIR)/../uclibc/lib/libpthread
UCC_DIR := $(PKGDIR)/../uclibc/lib/uclibc

PC_EXTRA = Link_DynLinker= rom/libld-l4.so

DIR_amd64 := x86_64
DIR_x86   := i386
DIR_arm   := arm
DIR_arm64 := arm64
DIR_ppc32 := powerpc
DIR_sparc := sparc
DIR_mips  := mips


vpath %.c $(CON_DIR)/ldso
vpath %.S $(CON_DIR)/ldso/$(DIR_$(ARCH))
vpath aeabi_read_tp-v6p.S     $(UCC_DIR)/ARCH-arm
vpath aeabi_read_tp_generic.c $(UCC_DIR)/ARCH-arm
vpath ssp-l4.c                $(UCC_DIR)

PRIVATE_INCDIR-x86 := $(CON_DIR)/../libc
PRIVATE_INCDIR := $(PRIVATE_INCDIR-$(ARCH)) \
                  $(CON_DIR)/ldso/$(DIR_$(ARCH)) \
		  $(SRC_DIR)           \
		  $(CON_DIR)/include \
		  $(PTH_DIR)/src \
		  $(PTH_DIR)/src/sysdeps/$(DIR_$(ARCH))


TARGET := libld-l4.so
SRC_C  := ldso.c fixup.c string.c
SRC_CC := syscalls.cc vfs.cc
#SRC_S_arm-l4f  += aeabi_read_tp-v6p.S
SRC_C_arm-l4f   += aeabi_read_tp_generic.c
SRC_S  := resolve.S

CFLAGS_string.c = -fvisibility=hidden
CFLAGS_fixup.c = -fvisibility=hidden

CPPFLAGS := -DNOT_IN_libc -DIS_IN_rtld \
            -DLDSO_ELFINTERP=\"$(DIR_$(ARCH))/elfinterp.c\" \
            -include libc-symbols.h -DUCLIBC_LDSO=\"libld-l4.so\" \
            -DUCLIBC_RUNTIME_PREFIX=\"/\" \
	    -D__LDSO_SEARCH_INTERP_PATH__=1 \
            -DIS_IN_rtld -D__LIBDL_SHARED__ -DSHARED

DEFINES_x86-l4f   += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_amd64-l4f += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_arm-l4f   += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_arm64-l4f += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_sparc-l4f += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_mips-l4f  += -DUSE_TLS=1 -DUSE___THREAD=1

LDFLAGS  =  -nocrt1 -Bsymbolic -Bstatic --warn-common --sort-common \
            --no-undefined -e _start -z defs -z now \
            --discard-locals --discard-all   \
            --gc-sections -ll4sys-direct $(GCCLIB)

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
CXXFLAGS := $(filter-out -Wmissing-declarations, $(CXXFLAGS))
CXXFLAGS += -DL4_NO_RTTI -fno-rtti -fno-exceptions -fvisibility=hidden \
            -fno-threadsafe-statics -O3

CPPFLAGS := $(filter-out -DL4SYS_USE_UTCB_WRAP=1, $(CPPFLAGS))

