PKGDIR ?= ..
L4DIR  ?= $(PKGDIR)/../../..

PKGNAME         = libgcc_eh
CONTRIB_HEADERS = y
INCSRC_DIR      = $(OBJ_DIR)/../include-gen/src
TARGET          = $(if $(BID_COMPILER_IS_GCC),unwind.h)

include $(L4DIR)/mk/include.mk
