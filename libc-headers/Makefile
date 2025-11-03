PKGDIR ?= .
L4DIR  ?= $(PKGDIR)/../../..

LIBCSRC_DIR = $(PKGDIR)/../libc/uclibc-ng

all:: include

include $(LIBCSRC_DIR)/make_vars.mk

vpath %.h $(LIBCSRC_DIR)/libc/ARCH-$(BUILD_ARCH)/include
vpath %.h $(LIBCSRC_DIR)/libc/ARCH-all/include

include $(LIBCSRC_DIR)/target_headers.mk


HEADER_DIR  := $(OBJ_DIR)/install_includes
DST_HEADERS := $(addprefix $(HEADER_DIR)/,$(HDR_common) $(HDR_$(BUILD_ARCH)))
GEN_HEADERS := $(addprefix $(HEADER_DIR)/,$(GEN_common) $(GEN_$(BUILD_ARCH)))

$(DST_HEADERS): $(HEADER_DIR)/%: % FORCE $(LIBCSRC_DIR)/target_headers.mk \
                                         $(LIBCSRC_DIR)/make_vars.mk
	$(VERBOSE)[ -d $(@D) ] || $(MKDIR) $(@D)
	$(VERBOSE)$(LN) -sf $(abspath $<) $@

$(HEADER_DIR): $(GENERAL_D_LOC) $(SRC_DIR)/Makefile $(LIBCSRC_DIR)/make_vars.mk \
               $(LIBCSRC_DIR)/target_headers.mk
	$(VERBOSE)$(RM) -r $@
	$(VERBOSE)install -d $@
	@$(INSTALL_LINK_MESSAGE)
	$(VERBOSE)$(MAKE) $(DST_HEADERS) $(GEN_HEADERS)

include: $(HEADER_DIR)
	$(VERBOSE)INCSRC_DIR=$(HEADER_DIR) $(MAKE) -f $(SRC_DIR)/Makefile.install
	$(VERBOSE)$(MAKE) -C $(LIBCSRC_DIR)/libpthread/include

clean cleanall::
