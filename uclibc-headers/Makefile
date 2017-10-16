PKGDIR ?= .
L4DIR  ?= $(PKGDIR)/../../..

LIBCSRC_DIR = $(PKGDIR)/../uclibc/lib/uclibc

all:: include

include $(LIBCSRC_DIR)/make_vars.mk

HEADERS_LST := $(LIBCSRC_DIR_ABS)/target_headers.lst
DST_HEADERS := $(shell cat $(HEADERS_LST))
HEADER_DIR  := $(OBJ_DIR)/install_includes

# just a few additions, handle them here directly
DST_HEADERS-arm     := bits/arm_asm.h
DST_HEADERS-arm     += bits/arm_bx.h
DST_HEADERS-sparc   := linux/sockios.h asm/traps.h
DST_HEADERS-ppc32   := asm/sockios.h asm/ioctl.h
DST_HEADERS-mips    := asm/sockios.h asm/ioctl.h asm-generic/ioctl.h \
                       asm/param.h asm/sgidefs.h sgidefs.h \
                       sys/asm.h sys/regdef.h sys/tas.h

DST_HEADERS += $(DST_HEADERS-$(BUILD_ARCH))

vpath %.h $(LIBCSRC_DIR)/ARCH-$(BUILD_ARCH)/include
vpath %.h $(LIBCSRC_DIR)/ARCH-all/include
vpath %.h $(CONTRIB_DIR)/libc/sysdeps/linux/$(UCLIBC_ARCH)
vpath %.h $(CONTRIB_DIR)/libc/sysdeps/linux/common
vpath %.h $(CONTRIB_DIR)/include

all:: $(HEADERS_LST)

DST_HEADERS   := $(addprefix $(HEADER_DIR)/,$(DST_HEADERS))

$(DST_HEADERS): $(HEADER_DIR)/%: % FORCE
	$(VERBOSE)[ -d $(@D) ] || $(MKDIR) -p $(@D)
	$(VERBOSE)$(LN) -sf $(call absfilename,$<) $@

$(HEADER_DIR): $(GENERAL_D_LOC) $(SRC_DIR)/Makefile
	$(VERBOSE)$(RM) -r $@
	$(VERBOSE)install -d $@
	@$(INSTALL_LINK_MESSAGE)
	$(VERBOSE)$(MAKE) $(MKFLAGS) $(DST_HEADERS)

include: $(HEADER_DIR)
	$(VERBOSE)INCSRC_DIR=$(HEADER_DIR)         \
	  $(MAKE) $(MKFLAGS)  PKGDIR=$(PKGDIR_ABS) \
	          -f $(SRC_DIR)/Makefile.install
	$(VERBOSE)PKGDIR=$(LIBCSRC_DIR)/../libpthread/include \
	  $(MAKE) $(MKFLAGS) -C $(LIBCSRC_DIR)/../libpthread/include

clean cleanall:
