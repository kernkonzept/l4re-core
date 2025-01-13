PKGDIR		?= .
L4DIR		?= $(PKGDIR)/../../..

SYSTEMS		= $(SYSTEMS_PLAIN)

# Avoid circular dependency, the other parts of the compiler-runtime require
# ldscripts, i.e. this package.
COMPILER_RT = compiler-rt-crt

LDSCRIPTS_CRT_BEGIN_PC = $(if $(CONFIG_COMPILER_RT_USE_LIBGCC),%{static:%:if-exists(%:search(crtbeginT.o %(libdir)) %:search(crtbegin.o %(libdir)));shared|static-pie:%:search(crtbeginS.o %(libdir));:%:search(crtbegin.o %(libdir))}) \
                         $(if $(CONFIG_COMPILER_RT_USE_LIBCLANG_RT),%:search(crtbegin.o %(libdir))) \

LDSCRIPTS_CRT_END_PC = $(if $(CONFIG_COMPILER_RT_USE_LIBGCC),%:search(%{shared|static-pie:crtendS.o;:crtend.o} %(libdir))) \
                       $(if $(CONFIG_COMPILER_RT_USE_LIBCLANG_RT),%:search(crtend.o %(libdir))) \

define PC_EXTRA
Link_Start= %{nocrt1|shared:; :%:search(crt1%{static:;static-pie:.p;:.s}.o %(libdir))} %:search(crti%{shared:.s}.o %(libdir)) $(LDSCRIPTS_CRT_BEGIN_PC)
Link_End=   $(LDSCRIPTS_CRT_END_PC) %:search(crtn%{shared:.s}.o %(libdir))
endef

LD_SCRIPTS      = main_stat.ld main_rel.ld main_dyn.ld \
                  $(if $(CONFIG_BID_PIE),main_pie.ld)
PC_FILENAME    := ldscripts
PC_LIBS        :=

# Create the wrapper binaries for programs and shared libs from uclibc
# source files (assembly).
# This is done here bacause l4sys.so already depends on the wrappers and must
# not depend on the whole libc package.
#
# to find the uclibc files
UCLIBC_ARCH_x86   := i386
UCLIBC_ARCH_arm   := arm
UCLIBC_ARCH_arm64 := aarch64
UCLIBC_ARCH_amd64 := x86_64
UCLIBC_ARCH_mips  := mips
UCLIBC_ARCH_ppc32 := powerpc
UCLIBC_ARCH_riscv := riscv
UCLIBC_ARCH_sparc := sparc
UCLIBC_BASE       = $(PKGDIR)/../uclibc
UCLIBC_SYSDEPS    = $(UCLIBC_BASE)/lib/contrib/uclibc/libc/sysdeps/linux/$(UCLIBC_ARCH_$(ARCH))

PRIVATE_INCDIR    = $(PKGDIR) $(UCLIBC_SYSDEPS)
TARGET            =

vpath %.S $(UCLIBC_SYSDEPS)

INSTALL_TARGET	= $(LD_SCRIPTS) crti.o crtn.o crt1.o crti.s.o crtn.s.o crt1.s.o crt1.p.o
DEFINES_crt1.s.o = -fPIE -DL_Scrt1
DEFINES_crt1.p.o = -fPIE -DL_rcrt1

REQUIRES_LIBS_all_lib :=

DEPS += $(foreach file,crti_s.o kip_addr.o, $(call BID_dot_fname,$(file)).cmd)

include $(L4DIR)/mk/lib.mk

$(eval $(call BID_MAKE_RULE_template, crti_s.o, crti.S,AS))
$(eval $(call BID_MAKE_RULE_template, kip_addr.o, kip_addr.S,AS))
$(eval $(call BID_MAKE_RULE_template, crt1.p.o, crt1.S,AS))

$(eval $(call make_per_obj_vars,crti.s.o,crti.s.o,crti.S))
$(eval $(call make_per_obj_vars,crti_s.o,crti_s.o,crti.S))
$(eval $(call make_per_obj_vars,crt1.o,crt1.o,crt1.S))
$(eval $(call make_per_obj_vars,crt1.p.o,crt1.p.o,crt1.S))
$(eval $(call make_per_obj_vars,crt1.s.o,crt1.s.o,crt1.S))
$(eval $(call make_per_obj_vars,crtn.o,crtn.o,crtn.S))
$(eval $(call make_per_obj_vars,crtn.s.o,crtn.s.o,crtn.S))

$(eval $(call make_per_obj_vars,crti.o,kip_addr.o,kip_addr.S))

DEFINES_kip_addr.o = -DL4_KIP_ADDR=$(L4_KIP_ADDR)

crti.o: crti_s.o $(if $(CONFIG_MMU),kip_addr.o)
	@$(GEN_MESSAGE)
	$(VERBOSE)$(LD) -r -o $@ $^

# we need to track pre-processing flags of ldscripts manually
.ldscripts-change-tracker: FORCE
	$(VERBOSE)echo -e "" \
	    "LD=$(LD)\n" \
	    "CONFIG_BID_LD_EMIT_UNWIND=$(CONFIG_BID_LD_EMIT_UNWIND)\n" \
	    "CFLAGS=$(CFLAGS)\n" \
	    "CONFIG_BID_STATIC_HEAP=$(CONFIG_BID_STATIC_HEAP)\n" \
	    "CONFIG_BID_STATIC_STACK=$(CONFIG_BID_STATIC_STACK)\n" \
	    "CONFIG_MMU=$(CONFIG_MMU)" > $@.tmp
	$(VERBOSE)$(call move_if_changed,$@,$@.tmp)

%.ld: $(PKGDIR)/ARCH-$(ARCH)/%.ld $(SRC_DIR)/Makefile $(SRC_DIR)/generic.h .ldscripts-change-tracker
	@$(GEN_MESSAGE)
	$(VERBOSE)$(CPP) -x c -nostdinc -include $(SRC_DIR)/generic.h \
	      -DCONFIG_BID_LD_EMIT_UNWIND=$(if $(CONFIG_BID_LD_EMIT_UNWIND),1,0) \
	      -DCONFIG_BID_STATIC_HEAP=$(if $(CONFIG_BID_STATIC_HEAP),1,0) \
	      -DCONFIG_BID_STATIC_STACK=$(if $(CONFIG_BID_STATIC_STACK),1,0) \
	      -DCONFIG_MMU=$(if $(CONFIG_MMU),1,0) \
	      -Wno-trigraphs -U$(UCLIBC_ARCH_$(ARCH)) $(CFLAGS) -P \
	      $(if $(filter lld,$(BID_LD_TYPE)),-DLD_LLD=1) $< -o $@

clean::
	$(VERBOSE)$(RM) *.ld
