PKGDIR ?= ..
L4DIR  ?= $(PKGDIR)/../../..

LIBCSRC_DIR ?= $(PKGDIR)/../uclibc/lib/uclibc

include $(LIBCSRC_DIR)/make_vars.mk

PC_FILENAME := libdl

vpath %.c $(CONTRIB_DIR)/ldso/libdl
vpath %.S $(CONTRIB_DIR)/ldso/ldso/$(LIBC_ARCH)

PRIVATE_INCDIR-x86 := $(CONTRIB_DIR)/libc
PRIVATE_INCDIR  = $(PRIVATE_INCDIR-$(ARCH)) \
                  $(OBJ_BASE)/include/uclibc \
                  $(CONTRIB_DIR)/ldso/ldso/$(LIBC_ARCH) \
                  $(SRC_DIR)           \
                  $(CONTRIB_DIR)/ldso/include \
                  $(PTHREAD_INCDIR)

TARGET := libdl.a libdl.so

SOVERSION_libdl.so := 1.0.50

SRC_S_libdl.a := resolve.S
SRC_C := libdl.c #dl-iterate-phdr.c dl-support.c


CPPFLAGS := -DNOT_IN_libc -DIS_IN_libdl \
            -DLDSO_ELFINTERP=\"../ldso/$(LIBC_ARCH)/elfinterp.c\" \
            -include libc-symbols.h -DUCLIBC_LDSO=\"libld-l4.so.1\" \
            -DUCLIBC_RUNTIME_PREFIX=\"/\"

DEFINES_x86-l4f   += -DUSE_TLS=1
DEFINES_amd64-l4f += -DUSE_TLS=1
DEFINES_arm-l4f   += -DUSE_TLS=1
DEFINES_arm64-l4f += -DUSE_TLS=1
DEFINES_mips-l4f  += -DUSE_TLS=1
DEFINES_riscv-l4f += -DUSE_TLS=1

LDFLAGS = -lld-l4
#LDFLAGS  =  -fini dl_cleanup

EXTRA_LIBS := -lld-l4

include $(L4DIR)/mk/lib.mk
PICFLAGS += -DSHARED

ldso.1.o: ldso.o
	$(OBJCOPY) --weaken $< $@

CXXFLAGS := -DL4_NO_RTTI -fno-rtti -fno-exceptions

ifeq ($(BID_COMPILER_TYPE),clang)
WARNINGS += -Wno-gnu-designator
endif

# Suppress false-positive warnings
WARNINGS += -Wno-bad-function-cast
