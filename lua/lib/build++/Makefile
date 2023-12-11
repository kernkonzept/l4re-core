PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

TARGET       := liblua++.a liblua++.so
PC_FILENAME  := lua++

include $(PKGDIR)/lib/build/Makefile

$(GENERAL_D_LOC): $(PKGDIR)/lib/build/Makefile

PKGNAME_DIRNAME := lua-c++

# Difference to other version

# WARNINGS EXCEPTION: CFLAGS is used here for C++ files (instead of the usual
# CXXFLAGS), therefore we filter out C-only warnings
CFLAGS := $(filter-out $(CFLAGS_L4_GENERIC) $(CFLAGS_C99) \
                       -Wstrict-prototypes -Wmissing-prototypes,$(CFLAGS))
CFLAGS += $(CXXFLAGS_L4_GENERIC) -x c++
