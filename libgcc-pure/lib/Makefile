PKGDIR	?= ..
L4DIR	?= $(PKGDIR)/../../..

SYSTEMS        = $(SYSTEMS_PLAIN)
PC_LIBS        = -lgcc
LIBGCC_LIB_DIR = $(PKGDIR)/../libgcc/lib
LIBGCC_DO_SHARED = $(CONFIG_MMU)

include $(LIBGCC_LIB_DIR)/Makefile.libgcc
