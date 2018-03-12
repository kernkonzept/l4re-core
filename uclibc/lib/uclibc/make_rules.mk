# vi:ft=make

CPPFLAGS	+= -nostdinc -include \
		  $(CONTRIB_DIR)/include/libc-symbols.h
CFLAGS		+= -fno-builtin $(GCCNOSTACKPROTOPT)
CFLAGS          += -DUCLIBC_INTERNAL
# CFLAGS	+= -std=iso9899:199901
DEFINES		+= -DNDEBUG -D_LIBC -D__UCLIBC_CTOR_DTOR__
WARNINGS	= -Wall -Wstrict-prototypes

# for building the C library we access internal headers
PRIVATE_INCDIR += $(CONTRIB_DIR)/libc/sysdeps/linux/$(UCLIBC_ARCH)
PRIVATE_INCDIR += $(CONTRIB_DIR)/libc/sysdeps/linux
# here we cheat a little and allow ../ includes from internal headers
PRIVATE_INCDIR += $(LIBC_DST_DIR)/libc $(PTHREAD_INCDIR)

# the inet subsystem needs the RESOLVER define to be set
CPPFLAGS_$(LIBC_DST_DIR)/libc/inet += -DRESOLVER="\"resolv.c\""

# elf subsystem may access ldso internals
CPPFLAGS_$(LIBC_DST_DIR)/libc/misc/elf += $(LDSO_INC)

# pthread support from libpthread uses libpthread internals
CPPFLAGS_$(LIBCSRC_DIR_ABS)/../$(PTHOBJ_PFX) += $(addprefix -I,$(PTHREAD_INCDIR))

# setup search paths for our sources
vpath %.c  $(LIBC_DST_DIR)
vpath %.cc $(LIBC_DST_DIR)
vpath %.S  $(LIBC_DST_DIR)
# libpthread support
vpath $(PTHOBJ_PFX)/% $(LIBCSRC_DIR_ABS)/..


# NOTE the two newlines in the define are essential!!
define NEWLINE


endef


define add_source_file_x
  UCLIBC_SRC_$(1)+=$(2)
endef

# add a source file to the BID list of sources
add_source_file = $(if $(filter %.c,$(1)),  $(eval $(call add_source_file_x,C$(2),$(1))), \
                  $(if $(filter %.cc,$(1)), $(eval $(call add_source_file_x,CC$(2),$(1))), \
                  $(if $(filter %.S,$(1)),  $(eval $(call add_source_file_x,S$(2),$(1))), \
                  $(if $(filter %.h,$(1)),  $(eval $(call add_source_file_x,H$(2),$(1))), \
                  $(error unknown source file: $(1))))))

# generate the search path value for source files
gen_search_path = $(LIBC_DST_DIR)/$(1)/$(UCLIBC_ARCH) \
                  $(LIBC_DST_DIR)/$(1)/generic        \
                  $(LIBC_DST_DIR)/$(1)/common         \
                  $(LIBC_DST_DIR)/$(1)

# search for a .c, a .S, or a .cc file for the given basename
search_source_file = $(or $(firstword $(foreach d,$(1),$(wildcard $(d)/$(2).[cS] $(d)/$(2).cc))), \
                          $(patsubst %.c,%$(suffix $(2)).c,$(firstword $(wildcard $(addsuffix /$(basename $(2)).c,$(1))))), \
                          $(patsubst %.h,%$(suffix $(2)).h,$(firstword $(wildcard $(addsuffix /$(basename $(2)).h,$(1))))), \
                          $(error source file for $(2) not found))

# arg 1: directory of the subsystem (e.g., libc/string)
# arg 2: the basename of the file to look for (e.g. memcpy, for memcpy.c, memcpy.cc, or
#        memcpy.S).
# arg 3: the additional suffix for the SRC_<X> variable
define HANDLE_file
  $(call add_source_file,$(subst $(LIBC_DST_DIR)/,,$(call search_source_file,$(1),$(2))),$(3))
endef

HANDLE_dir = $(foreach f,$(subst $(NEWLINE), ,$(SRC_$(1)$(2))),$(call HANDLE_file,$(call gen_search_path,$(1)),$(f),$(3)))

include $(L4DIR)/mk/rules.inc

#####################
# Generate pseudo rules for compiling the a template source file to a specific
# object file using a command line define -DL_<target>.  This is for example used
# for libm, where all kinds of functions are compiled from float_wrappers.c.
#
# arg 1: subsystem name (e.g. libm) -- <subsys>
# arg 2: module name (e.g. float, or double) -- <module>
#
# Uses:
#  SRC_<subsys><module> as list of target objects to generate
#  SRC_<subsys><module>_src as the source file for the objects
#
define OBJS_FN_templ
  OBJS_FN_$(1)$(2) = $(addprefix $(1)/,$(patsubst %,%.o,$(subst $(NEWLINE), ,$(SRC_$(1)$(2)))))
  UCLIBC_SRC_C += $$(OBJS_FN_$(1)$(2):.o=.c)
  $(call BID_MAKE_RULE_template,$$(OBJS_FN_$(1)$(2)): %.o,$(1)/$$(SRC_$(1)$(2)_src),C,-DL_$$(patsubst %.o,%,$$(notdir $$@)))
  $(call BID_MAKE_RULE_template,$$(OBJS_FN_$(1)$(2):.o=.s.o): %.s.o,$(1)/$$(SRC_$(1)$(2)_src),C,$$(PICFLAGS) -DL_$$(patsubst %.s.o,%,$$(notdir $$@)))
endef


$(eval $(call BID_GENERATE_DEFAULT_MAKE_RULES,%.__DO_UNLOCKED,%.c,C,-D__DO_UNLOCKED))


define PROCESS_src_lists
  # SRC_<dir> (e.g. SRC_libc/string)
  $(foreach d,$(1),$(call HANDLE_dir,$(d),,))
  # SRC_<dir>_<build_arch> (e.g. SRC_libc/string_arm)
  $(foreach d,$(1),$(call HANDLE_dir,$(d),_$(BUILD_ARCH),))
  # SRC_<dir>_<target> (e.g. SRC_libc/string_libuc_c.a)
  $(foreach t,$(TARGET),$(foreach d,$(1),$(call HANDLE_dir,$(d),_$(t),_$(t))))
  # SRC_<dir>_<submodule> (e.g. SRC_libc/stdio_wchar)
  $(foreach t,$(2),$(foreach d,$(1),$(call HANDLE_dir,$(d),_$(t),)))
endef

define PROCESS_template_src
  $(foreach d,$(1),$(foreach m,$(2),$(eval $(call OBJS_FN_templ,$(d),_$(m)))))
endef
