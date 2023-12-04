PKGDIR ?= .
L4DIR  ?= $(PKGDIR)/../../..

STDCXX_PKG_DIR =  $(PKGDIR)/../libstdc++-v3
SUPCXX_PKG_DIR =  $(PKGDIR)/../libsupc++

TARGET          = libsupc++_minimal.a
PC_FILENAME     = libsupc++_minimal
PC_LIBS		= -lsupc++_minimal
PC_EXTRA        = Link_Libs= %{static|static-pie:-lsupc++_minimal}
REQUIRES_LIBS   = libc_minimal
DEFINES        += -DL4_MINIMAL_LIBC

include $(PKGDIR)/../libsupc++/build/Makefile

# WARNING EXCEPTION: Since we do not want to modify the libsupc++ code, the
# warnings have been manually evaluated and found to not be problematic.
WARNINGS_eh_call.cc = -Wno-missing-declarations
# WARNING EXCEPTION: Since we do not want to modify the libsupc++ code, the
# warnings have been manually evaluated and found to not be problematic.
WARNINGS_bad_array_length.cc = -Wno-missing-declarations

vpath %.cc $(SUPCXX_PKG_DIR)/build
