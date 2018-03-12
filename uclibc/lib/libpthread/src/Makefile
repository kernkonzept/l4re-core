PKGDIR     ?= ../../..
L4DIR      ?= $(PKGDIR)/../../..

TARGET      = libpthread.a libpthread.so

PC_FILENAME = libpthread
# this will proper work with pkg-config >= 0.28, for older versions
# pkg-config's reordering will not harm but std::thread will not work
PC_LIBS_libpthread := --whole-archive -lpthread --no-whole-archive

CONTRIBDIR = $(PKGDIR)/../uclibc/lib/contrib/uclibc
LDSODIR    = $(CONTRIBDIR)/ldso/ldso

DIR_amd64 := x86_64
DIR_x86   := i386
DIR_arm   := arm
DIR_mips  := mips
DIR_ppc32 := powerpc
DIR_sparc := sparc

SRC_CC      = manager.cc l4.cc
SRC_C      += spinlock.c mutex.c condvar.c rwlock.c errno.c specific.c \
              semaphore.c attr.c barrier.c join.c pthread.c \
              cancel.c ptcleanup.c errno-loc.c signals.c \
              sysdeps/$(DIR_$(ARCH))/pspinlock.c
SRC_C_libpthread.a += libc_pthread_init.c
SRC_S       = tramp-$(ARCH).S
SO_INIT     = so_init-$(ARCH)
WARNINGS    = $(WARNINGS_MEDIUM)

vpath %.c $(SRC_DIR)/sysdeps/generic


INCDIR-x86   = $(SRC_DIR)/sysdeps/i386/i686 $(SRC_DIR)/sysdeps/i386 \
               $(CONTRIBDIR)/libc \
               $(LDSODIR)/$(DIR_$(ARCH))
INCDIR-amd64 = $(SRC_DIR)/sysdeps/$(DIR_$(ARCH))
INCDIR-arm   = $(SRC_DIR)/sysdeps/arm
INCDIR-mips  = $(SRC_DIR)/sysdeps/mips
INCDIR-ppc32 = $(SRC_DIR)/sysdeps/powerpc \
               $(CONTRIBDIR)/libpthread/linuxthreads/sysdeps/powerpc
INCDIR-sparc = $(SRC_DIR)/sysdeps/sparc \
               $(CONTRIBDIR)/libpthread/linuxthreads/sysdeps/sparc

PRIVATE_INCDIR = $(SRC_DIR) $(INCDIR-$(ARCH)) $(CONTRIBDIR)/ldso/include

DEFINES   := -DL4_THREAD_SAFE -D__USE_XOPEN2K -D_POSIX_C_SOURCE=200112L \
             -D_GNU_SOURCE=1 -D__NO_WEAK_PTHREAD_ALIASES=1 \
	     -D_LIBC=1 -DIS_IN_libpthread=1 -DNOT_IN_libc=1

DEFINES_x86-l4f   := -DUSE___THREAD=1 
DEFINES_amd64-l4f := -DUSE___THREAD=1 
DEFINES_arm-l4f   := -DUSE___THREAD=1 
DEFINES_mips-l4f  := -DUSE___THREAD=1 
DEFINES_ppc32-l4f := -DUSE___THREAD=1 
DEFINES_sparc-l4f := -DUSE___THREAD=1 

CFLAGS    += -include $(CONTRIBDIR)/include/libc-symbols.h
CXXFLAGS  += -include $(CONTRIBDIR)/include/libc-symbols.h
CXXFLAGS  += -fno-exceptions

include $(L4DIR)/mk/lib.mk
CRT0 = $(CRTI_so) $(SO_INIT).s.o $(CRTBEGIN_so) $(CRT1_so)

PICFLAGS += -DSHARED

libpthread.so: $(SO_INIT).s.o
