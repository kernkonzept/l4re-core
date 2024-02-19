LIBCSRC_DIR ?= $(SRC_DIR)

include $(LIBCSRC_DIR)/make_vars.mk

LIBC_SRC_DIRS := $(CONTRIB_DIR)/libc \
                 $(LIBCSRC_DIR_ABS)/ARCH-all/libc #$(LIBCSRC_DIR)/ARCH-$(BUILD_ARCH)/libc

LIBC_SRC_DIRS += $(CONTRIB_DIR)/libm \
                 $(CONTRIB_DIR)/libcrypt

LIBC_DST_DIR  := $(OBJ_DIR)/src

$(LIBC_DST_DIR)/.links-done: $(SRC_DIR)/Makefile $(LIBCSRC_DIR)/src_rules.mk \
                             $(LIBCSRC_DIR)/contrib_files.mk $(LIBCSRC_DIR)/make_vars.mk
	$(VERBOSE)$(MKDIR) $(LIBC_DST_DIR)
	$(VERBOSE)$(CP) -sfr $(LIBC_SRC_DIRS) $(LIBC_DST_DIR)
	$(VERBOSE)touch $@

include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(LIBC_DST_DIR)/.links-done
