# vi:ft=make

CPPFLAGS	+= -nostdinc -include \
		  $(CONTRIB_DIR)/include/libc-symbols.h $(LDSO_INC)
CFLAGS		+= -fno-builtin $(GCCNOSTACKPROTOPT)
CFLAGS          += -DUCLIBC_INTERNAL
CXXFLAGS        += -fno-builtin $(GCCNOSTACKPROTOPT)
# CFLAGS	+= -std=iso9899:199901
DEFINES		+= -DNDEBUG -D_LIBC -D__UCLIBC_CTOR_DTOR__
WARNINGS	= -Wall -Wstrict-prototypes $(call bid_flag_variants,WARNINGS)
ifeq ($(BID_COMPILER_TYPE),clang)
  # WARNING EXCEPTION: only warnings are about checking parameters marked with
  # the nonnull attribute for NULL, but this is defensive programming.
  WARNINGS += -Wno-tautological-pointer-compare
  # WARNING EXCEPTION: only warnings are that converting a nonnull pointer to
  # bool always evaluates to true, but checking again is defensive programming.
  WARNINGS += -Wno-pointer-bool-conversion
  # WARNING EXCEPTION: only warnings are about function aliases that resolve
  # to the original function, when this original function is overridden, but
  # this concerns functions which are not being replaced outside of the library.
  $(foreach f,strchr strrchr strcmp memcmp mempcpy,\
    $(eval WARNINGS_$(f).c += -Wno-ignored-attributes))
  # WARNING EXCEPTION: only warnings concern possibly reduced performance due to
  # usage of wrong floating point types, where in fact suitable functions are
  # chosen according to type sizes.
  $(foreach f, e_lgamma_r e_scalb s_fdim s_fmax s_fmin s_ldexp, \
    $(eval WARNINGS_$(f).c += -Wno-double-promotion))
  # WARNING EXCEPTION: the float equality checks are necessary during the
  # computation of floating point arithmetic.
  $(foreach f,e_lgamma_r e_log e_log2 e_rem_pio2 k_rem_pio2 e_scalb s_log1p\
    s_nextafterf s_nextafter __strtofpmax __fp_range_check _fpmaxtostr,\
    $(eval WARNINGS_$(f).c += -Wno-float-equal))
  # WARNING EXCEPTION: warnings about comparisons of variables with differently
  # signed types are left in the math library: the variables are also used
  # with bitwise arithmetic, so the concrete bitwise layout is important.
  $(foreach f,e_pow e_sqrt s_ceil s_floor,\
    $(eval WARNINGS_$(f).c += -Wno-sign-compare))
  # WARNING EXCEPTION: only warning is due to a macro expansion where zero
  # is added to a null-pointer.
  WARNINGS__stdio.c += -Wno-null-pointer-arithmetic
  # WARNING EXCEPTION: The warning is useful but the generated code works
  # anyway as intended
  $(foreach s,S s.S,$(foreach f,bzero memcpy mempcpy memset strcspn strpbrk,\
    $(eval ASFLAGS_libc/string/x86_64/$(f).$(s) += -Wno-expansion-to-defined)))
else
  # WARNING EXCEPTION: checking a parameter marked with the nonnull attribute
  # for NULL is defensive programming.
  WARNINGS	+= -Wno-nonnull-compare
  # WARNING EXCEPTION: the function is only ever invoked with prec = 2 and thus
  # the values of jk (and by extension jz) are always positive. If that is the
  # case then the loops will initialize fq and iq and there will never be an
  # unitialized access to element 0 of fq.
  WARNINGS_k_rem_pio2.c += -Wno-maybe-uninitialized
  # WARNING EXCEPTION: the only two instances where the warning triggers are
  # well documented intentional uses of aliasing to convey library internal
  # information. The pointers are used for signaling a special condition and one
  # of them is replaced in the called function, so no aliasing use of the
  # pointers occurs.
  $(foreach f,_vfprintf _vfprintf_internal _vfwprintf_internal,\
    $(eval WARNINGS_$(f).c += -Wno-restrict))
endif

# for building the C library we access internal headers
PRIVATE_INCDIR += $(CONTRIB_SYSDEPS)/$(LIBC_ARCH)
PRIVATE_INCDIR += $(CONTRIB_SYSDEPS)
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
gen_search_path = $(LIBC_DST_DIR)/$(1)/$(LIBC_ARCH) \
                  $(LIBC_DST_DIR)/$(1)/generic      \
                  $(LIBC_DST_DIR)/$(1)/common       \
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
