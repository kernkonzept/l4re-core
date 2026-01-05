PKGDIR     ?= ../../..
L4DIR      ?= $(PKGDIR)/../../..

# The real code is in libc_pthread which is linked into libc.(a|so). There is
# an empty libpthread.a to satisfy applications that unconditionally link
# with libpthread.
TARGET      = libpthread.a libc_pthread.ofl libc_pthread.p.a

PC_FILENAMES               = libpthread libc_pthread
PC_LIBS_libpthread         = -lpthread
PC_LIBS_libc_pthread       = %{-link-libc:%{shared:--whole-archive -lc_pthread.p --no-whole-archive;:libc_pthread.ofl}}
PC_LIBS_PIC                =

LIBCSRC_DIR ?= $(PKGDIR)/musl

include $(LIBCSRC_DIR)/make_vars.mk

LDSODIR    = $(CONTRIB_DIR)/ldso/ldso

SRC_CC_libc_pthread   = manager.cc l4.cc
SRC_C_libc_pthread   += spinlock.c mutex.c condvar.c rwlock.c specific.c \
                        semaphore.c attr.c barrier.c join.c pthread.c \
                        cancel.c signals.c \
                        sysdeps/$(LIBC_ARCH)/pspinlock.c impl-musl.c
SRC_S_libc_pthread    = tramp-$(ARCH).S

SRC_C_libc_pthread.p.a  += $(SRC_C_libc_pthread)
SRC_CC_libc_pthread.p.a += $(SRC_CC_libc_pthread)
SRC_S_libc_pthread.p.a  += $(SRC_S_libc_pthread)

SRC_C_libc_pthread.ofl  += $(SRC_C_libc_pthread)
SRC_CC_libc_pthread.ofl += $(SRC_CC_libc_pthread)
SRC_S_libc_pthread.ofl  += $(SRC_S_libc_pthread)

WARNINGS    = $(WARNINGS_MEDIUM)

vpath %.c $(SRC_DIR)/sysdeps/generic


INCDIR-x86   = $(SRC_DIR)/sysdeps/i386/i686 $(SRC_DIR)/sysdeps/i386 \
               $(CONTRIB_DIR)/libc \
               $(LDSODIR)/$(LIBC_ARCH)
INCDIR-amd64 = $(SRC_DIR)/sysdeps/$(LIBC_ARCH)
INCDIR-arm   = $(SRC_DIR)/sysdeps/arm
INCDIR-arm64 = $(SRC_DIR)/sysdeps/aarch64
INCDIR-mips  = $(SRC_DIR)/sysdeps/mips
INCDIR-ppc32 = $(SRC_DIR)/sysdeps/powerpc \
               $(CONTRIB_DIR)/libpthread/linuxthreads/sysdeps/powerpc
INCDIR-riscv = $(SRC_DIR)/sysdeps/riscv

PRIVATE_INCDIR = $(SRC_DIR) $(INCDIR-$(ARCH)) $(CONTRIB_DIR)/arch/$(LIBC_ARCH)

DEFINES   := -DL4_THREAD_SAFE -D_GNU_SOURCE=1 -D__NO_WEAK_PTHREAD_ALIASES=1 \
             -D_LIBC=1 -DIS_IN_libpthread=1 -DNOT_IN_libc=1 -DL4_NO_RTTI=1 \
             -DUSE_TLS=1

CXXFLAGS  += -fno-exceptions

include $(L4DIR)/mk/lib.mk

PICFLAGS += -DSHARED
