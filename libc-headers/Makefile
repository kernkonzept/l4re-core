PKGDIR ?= .
L4DIR  ?= $(PKGDIR)/../../..

include $(L4DIR)/mk/Makeconf

LIBCSRC_DIR = $(PKGDIR)/../libc/$(CONFIG_L4_LIBC)

all:: include

include $(LIBCSRC_DIR)/make_vars.mk

vpath %.h $(LIBCSRC_DIR)/libc/ARCH-$(BUILD_ARCH)/include
vpath %.h $(LIBCSRC_DIR)/libc/ARCH-all/include

HEADER_DIR  := $(OBJ_DIR)/install_includes

include $(LIBCSRC_DIR)/target_headers.mk


DST_HEADERS := $(addprefix $(HEADER_DIR)/,$(HDR_common) $(HDR_$(BUILD_ARCH)))
GEN_HEADERS := $(addprefix $(HEADER_DIR)/,$(GEN_common) $(GEN_$(BUILD_ARCH)))

$(HEADER_DIR).include_tracker: $(DST_HEADERS)
	$(if $?,touch $@)

$(DST_HEADERS): $(HEADER_DIR)/%: % FORCE $(LIBCSRC_DIR)/target_headers.mk \
                                         $(LIBCSRC_DIR)/make_vars.mk
	$(VERBOSE)$(call lessfork_mkdir,$(@D))
	$(VERBOSE)[[ $< -ef $@ ]] || $(LN) -sf $(abspath $<) $@

$(HEADER_DIR): $(GENERAL_D_LOC) $(SRC_DIR)/Makefile $(LIBCSRC_DIR)/make_vars.mk \
               $(LIBCSRC_DIR)/target_headers.mk $(HEADER_DIR).include_tracker \
               $(foreach g,$(GEN_common) $(GEN_$(BUILD_ARCH)),$(GEN_DEPS_$g))
	$(VERBOSE)$(RM) -r $@
	$(VERBOSE)install -d $@
	@$(INSTALL_LINK_MESSAGE)
	$(VERBOSE)$(MAKE) $(DST_HEADERS) $(GEN_HEADERS)

include: $(HEADER_DIR)
	$(VERBOSE)INCSRC_DIR=$(HEADER_DIR) $(MAKE) -f $(SRC_DIR)/Makefile.install
	$(VERBOSE)$(MAKE) -C $(LIBCSRC_DIR)/libpthread
	$(VERBOSE)$(MAKE) -C $(PKGDIR)/../libpthread/include

clean cleanall::
