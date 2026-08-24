LIBCSRC_DIR ?= $(SRC_DIR)

include $(LIBCSRC_DIR)/make_vars.mk

LIBC_LINKS_STAMP := $(LIBC_DST_DIR)/.links-done-$(CONFIG_L4_LIBC)

# The files that were linked last time. $(file <...) keeps the newlines of the
# file and a prerequisite list is not split at newlines, so $(strip ...) them
# away, otherwise the whole list ends up as a single bogus prerequisite.
LIBC_LINKED_FILES := $(strip $(if $(wildcard $(LIBC_LINKS_STAMP)),\
                                  $(file <$(LIBC_LINKS_STAMP).list)))
# A file that vanished meanwhile has no rule to make it and would abort the
# build. Depend on FORCE instead, the link farm is recreated from scratch anyway.
LIBC_LINKED_GONE := $(filter-out $(wildcard $(LIBC_LINKED_FILES)),\
                                 $(LIBC_LINKED_FILES))

$(LIBC_LINKS_STAMP): $(SRC_DIR)/Makefile $(LIBCSRC_DIR)/../src_rules.mk\
                     $(LIBCSRC_DIR)/contrib_files.mk \
                     $(LIBCSRC_DIR)/make_vars.mk \
                     $(LIBCSRC_DIR)/sources.mk \
                     $(shell find $(LIBC_SRC_DIRS) -type f) \
                     $(wildcard $(LIBC_LINKED_FILES)) \
                     $(if $(LIBC_LINKED_GONE),FORCE)
	$(VERBOSE)$(RM) -r $(LIBC_DST_DIR)
	$(VERBOSE)$(MKDIR) $(LIBC_DST_DIR)
	$(VERBOSE)$(CP) -sfr $(LIBC_SRC_DIRS) $(LIBC_DST_DIR)
	$(VERBOSE)find $(LIBC_DST_DIR) -type l -exec realpath {} + >$@.list
	$(VERBOSE)touch $@

include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(LIBC_LINKS_STAMP)
