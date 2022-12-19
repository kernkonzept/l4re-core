PKGDIR	?= ..
L4DIR	?= $(PKGDIR)/../../..

TARGET         = libgcc-l4.a libgcc-l4.so
SRC_C_arm-l4f  = ARCH-arm/l4-atomic.c ARCH-arm/l4-atomic-64bit.c
SRC_S_arm-l4f  = ARCH-arm/atomic_ops_s.S
DEFINES        += -DIS_L4
LIBGCC_DO_SHARED = y
LIBGCC_LIB_DIR = $(SRC_DIR)

PC_FILENAMES      = libgcc libgcc-l4
PC_LIBS_libgcc    = -lgcc
PC_LIBS_libgcc-l4 = -lgcc-l4
INSTALL_TARGET   += libgcc-l4.a

CFLAGS_ARCH-arm/l4-atomic.c = -Wno-builtin-declaration-mismatch
CFLAGS_ARCH-arm/l4-atomic-64bit.c = -Wno-builtin-declaration-mismatch

include $(PKGDIR)/lib/Makefile.libgcc
