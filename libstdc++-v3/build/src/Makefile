PKGDIR ?= 	../..
L4DIR  ?= 	$(PKGDIR)/../../..

TARGET         ?= libstdc++.a libstdc++.so
PC_FILENAME    ?= libstdc++
PC_LIBS        ?= -lstdc++
REQUIRES_LIBS  ?= libsupc++
STDCXX_PKG_DIR ?= $(PKGDIR)

PRIVATE_INCDIR += $(STDCXX_PKG_DIR)/private_inc

# 1: target: will be prefixed with OBJ_DIR
# 2: source
define libstdcpp_link_rule
$(OBJ_DIR)/$1: $(realpath $2) $(if $(filter $(realpath $(OBJ_DIR)/$1),$(realpath $2)),,FORCE)
	$$(VERBOSE)$(MKDIR) $$(@D)
	$$(VERBOSE)$(LN) -sf $$< $$@
endef

STDCXX_IS_FULL := $(if $(STDCXX_IS_MINIMAL),,y)

include $(STDCXX_PKG_DIR)/contrib.inc
include $(STDCXX_PKG_DIR)/build/src/Makefile-$(STDCXX_CONTRIB_VERSION)
include $(L4DIR)/mk/lib.mk

$(GENERAL_D_LOC): $(STDCXX_PKG_DIR)/build/src/Makefile-$(STDCXX_CONTRIB_VERSION)

ifeq ($(BID_COMPILER_TYPE),clang)
# WARNING EXCEPTION: Clang does not understand GCC-specific attributes
WARNINGS += -Wno-ignored-attributes
# WARNING EXCEPTION: the usage of the C++11 feature inline namespaces is
# correctly marked with cxx11 abi_tag but Clang does not understand the
# abi_tag attribute
WARNINGS += -Wno-c++11-inline-namespace
# WARNING EXCEPTION: The warnings are correct but the code works as intended,
# no safety problem
WARNINGS += -Wno-inline-namespace-reopened-noninline
# WARNING EXCEPTION: libstdc++ enables in some parts of the code GCC-specific
# compiler warnings which Clang does not support.
WARNINGS += -Wno-unknown-warning-option
# WARNING EXCEPTION: The explicit instantiations have no effect but it is not a
# safety issue because it does not change the semantics
WARNINGS += -Wno-inline-explicit-instantiation
# WARNING EXCEPTION: The explicit instantiations have no effect but it is not a
# safety issue because it does not change the semantics
WARNINGS += -Wno-static-inline-explicit-instantiation
# WARNING EXCEPTION: The explicit instantiations have no effect but it is not a
# safety issue because it does not change the semantics
WARNINGS += -Wno-instantiation-after-specialization
# WARNING EXCEPTION: Unused functions are not a safety issue
WARNINGS += -Wno-unused-function
# WARNING EXCEPTION: The mismatch does not change the semantics
WARNINGS += -Wno-implicit-exception-spec-mismatch
# WARNING EXCEPTION: The function is explicitly marked as 'override'
WARNINGS_c++11/cxx11-ios_failure.cc += -Wno-overloaded-virtual
# WARNING EXCEPTION: Warning is only about Microsoft C++ ABI
WARNINGS_c++11/debug.cc += -Wno-mismatched-tags
endif

# WARNING EXCEPTION: The warnings have been manually evaluated and were found
# to be not problematic. In some cases, the declarations are not missing, just
# the names of the functions are mangled.
WARNINGS += -Wno-missing-declarations
# WARNING EXCEPTION: even though comparing doubles may be unsafe, comparing to
# 0 or infinity is not
WARNINGS += -Wno-float-equal
# WARNING EXCEPTION: unused parameters are due to conditional code in the
# function (which uses the variable)
WARNINGS += -Wno-unused-parameter
# WARNING EXCEPTION: it is not defined whether wchar_t is signed or unsigned,
# therefore checking it for >= 0 is sensible
WARNINGS_ctype_members.cc += -Wno-type-limits
# WARNING EXCEPTION: Single warning about a deprecated but anyway still
# supported libstdc++ feature.
WARNINGS_c++98/strstream.cc += -Wno-deprecated
# WARNING EXCEPTION: The floating point values are not altered by the
# conversion because the are first floored/cealed to an integral value
WARNINGS_bits/hashtable_policy.h += -Wno-float-conversion
# WARNING EXCEPTION: The floating point values are not altered by the
# conversion because the are first floored/cealed to an integral value
WARNINGS_c++11/hashtable_c++0x.cc += -Wno-float-conversion
# WARNING EXCEPTION: The loss of decimal places when calculating buffer sizes
# with a floating point factor is intentional
WARNINGS_c++17/memory_resource.cc += -Wno-float-conversion
# WARNING EXCEPTION: The loss of decimal places when calculating capacities
# with a floating point factor is intentional
WARNINGS_c++17/fs_path.cc += -Wno-float-conversion

vpath %.cc $(OBJ_DIR)
vpath %.cc $(STDCXX_CONTRIB_DIR)/config/io
vpath %.cc $(STDCXX_CONTRIB_DIR)/config/locale/generic

vpath %.cc $(STDCXX_CONTRIB_DIR)/src
