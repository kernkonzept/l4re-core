# vi:ft=make
CPPFLAGS  += -nostdinc -ffreestanding
CFLAGS    += -fno-builtin -std=c99 $(GCCNOSTACKPROTOPT)
CXXFLAGS  += -fno-builtin $(GCCNOSTACKPROTOPT)

WARNINGS    = -Wall -Wstrict-prototypes $(call bid_flag_variants,WARNINGS)

# Temporary disable noisy musl warnings
WARNINGS   += -Wno-strict-prototypes \
              -Wno-missing-prototypes \
              -Wno-unused-parameter \
              -Wno-unused-function \
              -Wno-parentheses \
              -Wno-unused-but-set-variable

# for building the C library we access internal headers
PRIVATE_INCDIR += $(LIBCSRC_DIR)/libc/overrides
PRIVATE_INCDIR += $(CONTRIB_DIR)/arch/$(LIBC_ARCH)
PRIVATE_INCDIR += $(CONTRIB_DIR)/arch/generic
# OBJ src internal?
#PRIVATE_INCDIR +=
PRIVATE_INCDIR += $(CONTRIB_DIR)/src/include
PRIVATE_INCDIR += $(CONTRIB_DIR)/src/internal
#OBJ_INCLUDE?
#PRIVATE_INCDIR +=
PRIVATE_INCDIR += $(CONTRIB_DIR)/include

PRIVATE_INCDIR_vfs.cc += $(CONTRIB_DIR)/include

# Include libpthread internal headers (should go to non-minimal Make.rules?)
PRIVATE_INCDIR += $(PTHREAD_INCDIR)

BID_ASM_FILE_EXTENSIONS += .s

# According to the POSIX specification of mmap it shall be possible to map a
# region of memory that is larger than the backing file. Musl makes use of this
# feature when loading shared libraries, but unfortunately the current L4Re mmap
# implementation does not support it.
# As a workaround, we use the alternative code paths that musl implements for
# DL_NOMMU_SUPPORT.
DEFINES_$(LIBC_DST_DIR)/ldso/dynlink.c += -DDL_NOMMU_SUPPORT=1

# setup search paths for our sources
vpath %.c  $(LIBC_DST_DIR)
vpath %.cc $(LIBC_DST_DIR)
vpath %.S  $(LIBC_DST_DIR)
vpath %.s  $(LIBC_DST_DIR)
# libpthread support (should go to non-minimal Make.rules?)
vpath $(PTHOBJ_PFX)/% $(LIBCSRC_DIR_ABS)

# for $(newline)
include $(L4DIR)/mk/util.mk

# add a source file to the BID list of sources
add_source_file = $(if $(filter %.c,$(1)),    $(eval LIBC_SRC_C$(2)  += $(1)), \
                  $(if $(filter %.cc,$(1)),   $(eval LIBC_SRC_CC$(2) += $(1)), \
                  $(if $(filter %.S %s,$(1)), $(eval LIBC_SRC_S$(2)  += $(1)), \
                  $(if $(filter %.h,$(1)),    $(eval LIBC_SRC_H$(2)  += $(1)), \
                  $(error unknown source file: $(1))))))

# generate the search path value for source files
gen_search_path = $(LIBC_DST_DIR)/src/$(1)/$(LIBC_ARCH) \
                  $(LIBC_DST_DIR)/src/$(1) $(LIBC_DST_DIR)/$(1)

# search for a .c, a .S, or a .cc file for the given basename
search_source_file = $(or $(firstword $(foreach d,$(1),$(wildcard $(d)/$(2)) $(wildcard $(d)/$(2).[cSs] $(d)/$(2).cc))), \
                          $(error source file for $(2) not found in '$1'))

# arg 1: directory of the subsystem (e.g., libc/string)
# arg 2: the basename of the file to look for (e.g. memcpy, for memcpy.c, memcpy.cc, or
#        memcpy.S).
# arg 3: the additional suffix for the SRC_<X> variable
define HANDLE_file
  $(call add_source_file,$(subst $(LIBC_DST_DIR)/,,$(call search_source_file,$(1),$(2))),$(3))
endef

HANDLE_dir = $(foreach f,$(subst $(newline), ,$(SRC_$(1)$(2))),$(call HANDLE_file,$(call gen_search_path,$(1)),$(f),$(3)))

include $(L4DIR)/mk/rules.inc

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
