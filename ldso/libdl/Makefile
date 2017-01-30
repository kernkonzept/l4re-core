PKGDIR ?= ..
L4DIR  ?= $(PKGDIR)/../../..

PC_FILENAME := libdl

CON_DIR := $(PKGDIR)/../uclibc/lib/contrib/uclibc
PTH_DIR := $(PKGDIR)/../uclibc/lib/libpthread

DIR_amd64 := x86_64
DIR_x86   := i386
DIR_arm   := arm
DIR_arm64 := arm64
DIR_ppc32 := powerpc
DIR_sparc := sparc
DIR_mips  := mips


vpath %.c $(CON_DIR)/ldso/libdl
vpath %.S $(CON_DIR)/ldso/ldso/$(DIR_$(ARCH))

PRIVATE_INCDIR-x86 := $(CON_DIR)/libc
PRIVATE_INCDIR := $(PRIVATE_INCDIR-$(ARCH)) \
                  $(CON_DIR)/ldso/ldso/$(DIR_$(ARCH)) \
		  $(SRC_DIR)           \
		  $(CON_DIR)/ldso/include \
		  $(PTH_DIR)/src \
		  $(PTH_DIR)/src/sysdeps/$(DIR_$(ARCH))

TARGET := libdl.a libdl.so

SRC_S_libdl.a := resolve.S
SRC_C := libdl.c #dl-iterate-phdr.c dl-support.c


CPPFLAGS := -DNOT_IN_libc -DIS_IN_libdl \
            -DLDSO_ELFINTERP=\"../ldso/$(DIR_$(ARCH))/elfinterp.c\" \
            -include libc-symbols.h -DUCLIBC_LDSO=\"libld-l4.so\" \
            -DUCLIBC_RUNTIME_PREFIX=\"/\"

DEFINES_x86-l4f   += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_amd64-l4f += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_arm-l4f   += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_arm64-l4f += -DUSE_TLS=1 -DUSE___THREAD=1
DEFINES_mips-l4f  += -DUSE_TLS=1 -DUSE___THREAD=1

LDFLAGS = -lld-l4
#LDFLAGS  =  -fini dl_cleanup

EXTRA_LIBS := -lld-l4

CRT0 :=

include $(L4DIR)/mk/lib.mk
PICFLAGS += -DSHARED

ldso.1.o: ldso.o
	$(OBJCOPY) --weaken $< $@

CXXFLAGS := -DL4_NO_RTTI -fno-rtti -fno-exceptions
