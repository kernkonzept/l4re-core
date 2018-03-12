PKGDIR	?= .
L4DIR	?= $(PKGDIR)/../../..

TARGET       = libstdc++_minimal.a
PC_FILENAME  = libstdc++_minimal
PC_LIBS      = -lstdc++_minimal
REQUIRES_LIBS= libsupc++_minimal

STDCXX_PKG_DIR = $(PKGDIR)/../libstdc++-v3
DEFINES += -DL4_MINIMAL_LIBC

include $(PKGDIR)/../libstdc++-v3/build/src/Makefile

$(GENERAL_D_LOC): $(PKGDIR)/../libstdc++-v3/build/src/Makefile
