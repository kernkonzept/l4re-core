PKGDIR ?= 	..
L4DIR  ?= 	$(PKGDIR)/../../..

STDCXX_PKG_DIR ?= $(PKGDIR_ABS)/../libstdc++-v3

CONTRIB_HEADERS = y
PKGNAME         = libstdc++-v3
INCSRC_DIR      = $(STDCXX_CONTRIB_DIR)/include
TARGET          = $(shell (cd $(INCSRC_DIR) && find . -type f -not -name 'Make*' -print))

LIBSUPC_H            := exception new typeinfo cxxabi.h
LIBSUPC_H-4.6_PLUS-  := exception_defines.h cxxabi-forced.h
LIBSUPC_H-4.6_PLUS-y := bits/exception_defines.h bits/cxxabi_forced.h cxxabi_forced.h bits/hash_bytes.h
LIBSUPC_H-4.7_PLUS-y += bits/atomic_lockfree_defines.h

ALLOCATOR_H        := config/allocator/new_allocator_base.h
CLOCALE_H          := config/locale/generic/c_locale.h
ATOMIC_WORD_H      := atomic_word.h
CPU_DEFINES_H      := cpu_defines.h
TIME_MEMBERS_H     := config/locale/generic/time_members.h
MESSAGES_MEMBERS_H := config/locale/generic/messages_members.h
BASIC_FILE_H       := config/io/basic_file_stdio.h
CSTDIO_H           := config/io/c_io_stdio.h
CMATH_TCC          := include/c_std/cmath.tcc


# FIX BID:

include $(L4DIR)/mk/include.mk
include $(STDCXX_PKG_DIR)/contrib.inc

ifeq ("$(STDCXX_0x)", "y")
  LIBSUPC_H            += initializer_list
  LIBSUPC_H-4.6_PLUS-  += exception_ptr.h nested_exception.h
  LIBSUPC_H-4.6_PLUS-y += bits/exception_ptr.h bits/nested_exception.h
endif
STDCXX_CSTD_HEADERS := \
  cassert cctype cerrno cfloat ciso646 climits clocale cmath csetjmp csignal \
  cstdarg cstddef cstdio cstdlib cstring ctime cwchar cwctype cstdint


STDCXX_CSTD_HEADERS += $(patsubst std_%.h,%,$(notdir $(wildcard $(INCSRC_DIR)/std/*)))

STDCXX_STD_DIRS     := std c_global c_std tr1


INSTALL_INCDIR_X = $(INSTALLDIR_LOCAL)/$(INSTALL_INC_PREFIX)
SRCDIR_X         = $(STDCXX_CONTRIB_DIR)

CTYPE_HEADERS := $(filter-out %/os_defines.h,$(wildcard $(SRCDIR_X)/config/os/generic/*))

install_header_x = $(MKDIR) $(dir $(INSTALL_INCDIR_X)/$(strip $(2))) && $(LN) -sf $(call absfilename,$(strip $(1)) $(INSTALL_INCDIR_X)/$(strip $(2)))
install_header   = $(MKDIR) $(dir $(INSTALL_INCDIR_X)/$(strip $(2))) && $(LN) -sf $(SRCDIR_X)/$(strip $(1)) $(INSTALL_INCDIR_X)/$(strip $(2))

install_config_header = $(call install_header, $(1), bits/$(strip $(2)))

cpu_header = $(word 1,$(wildcard $(SRCDIR_X)/$(CPU_H_arch)/$(strip $(1)) $(SRCDIR_X)/$(CPU_H_generic)/$(strip $(1))))

install_cpu_header = $(VERBOSE)$(if $(call cpu_header,$(1)),$(LN) -sf $(call cpu_header,$(1)) $(INSTALL_INCDIR_X)/bits/$(strip $(notdir $(1))))

LIBSUPC_H += $(foreach V,$(STDCXX_AVAILABLE_VERSIONS),$(LIBSUPC_H-$(V)_PLUS-$(STDCXX_CONTRIB_VERSION_$(V)_PLUS)))

LIBSUPC_HI = $(addprefix $(INSTALL_INCDIR_X)/, $(LIBSUPC_H))
STDCXX_CSTD_HEADERS_ABS = $(addprefix $(INSTALL_INCDIR_X)/, $(STDCXX_CSTD_HEADERS))


std_header_list = $(strip $(1)) std_$(strip $(1)).h

choose_std_header = $(word 1, $(wildcard $(foreach DIR,$(STDCXX_STD_DIRS),$(addprefix $(INCSRC_DIR)/$(DIR)/, $(call std_header_list, $(1))))))

$(STDCXX_CSTD_HEADERS_ABS): %: $(INSTALL_INCDIR_X)
	@$(INSTALL_LINK_MESSAGE)
	$(VERBOSE)$(LN) -sf $(call choose_std_header,$(notdir $@)) $@

$(LIBSUPC_HI): %: $(INSTALL_INCDIR_X)
	@$(INSTALL_LINK_MESSAGE)
	$(VERBOSE)$(call install_header, libsupc++/$(notdir $@), $(patsubst $(INSTALL_INCDIR_X)/%,%,$@))

$(INSTALL_INCDIR_X):
	@$(INSTALL_LINK_MESSAGE) $(INSTALL_INCDIR_X)
	$(VERBOSE)$(INSTALL) -d $(INSTALL_INCDIR_X)

all:: $(INSTALL_INCDIR_X) $(LIBSUPC_HI) $(STDCXX_CSTD_HEADERS_ABS)
	@$(INSTALL_LINK_MESSAGE)
	$(VERBOSE)$(call install_config_header, $(ALLOCATOR_H),     c++allocator.h)
	$(VERBOSE)$(call install_config_header, $(CLOCALE_H),       c++locale.h)
	$(call install_cpu_header, atomic_word.h)
	$(call install_cpu_header, cpu_defines.h)
	$(call install_cpu_header, cxxabi_tweaks.h)
	$(call install_cpu_header, opt/bits/opt_random.h)
	$(VERBOSE)$(foreach H,$(CTYPE_HEADERS), $(call install_header_x, $(H), bits/$(notdir $(H))); )
	$(VERBOSE)$(call install_config_header, $(TIME_MEMBERS_H),  time_members.h)
	$(VERBOSE)$(call install_config_header, $(MESSAGES_MEMBERS_H),  messages_members.h)
	$(VERBOSE)$(call install_config_header, $(BASIC_FILE_H), basic_file.h)
	$(VERBOSE)$(call install_config_header, $(CSTDIO_H), c++io.h)
	$(VERBOSE)$(call install_config_header, $(CMATH_TCC), cmath.tcc)
