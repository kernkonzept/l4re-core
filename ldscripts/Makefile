PKGDIR		?= .
L4DIR		?= $(PKGDIR)/../../..

SYSTEMS		= $(SYSTEMS_PLAIN)

# Page sizes for linker
MAXPAGESIZE_amd64    := 0x200000
COMMONPAGESIZE_amd64 := 0x1000

MAXPAGESIZE_x86      := 0x1000
COMMONPAGESIZE_x86   := 0x1000

MAXPAGESIZE_arm      := 0x1000
COMMONPAGESIZE_arm   := 0x1000

MAXPAGESIZE_ppc32    := 0x1000
COMMONPAGESIZE_ppc32 := 0x1000

LD_SCRIPTS      = main_stat.ld main_rel.ld main_dyn.ld

# Create the wrapper binaries for programs and shared libs from uclibc
# source files (assembly).
# This is done here bacause l4sys.so already depends on the wrappers and must
# not depend on the whole libc package.
#
# to find the uclibc files
UCLIBC_ARCH_x86   := i386
UCLIBC_ARCH_arm   := arm
UCLIBC_ARCH_amd64 := x86_64
UCLIBC_ARCH_mips  := mips
UCLIBC_ARCH_ppc32 := powerpc
UCLIBC_ARCH_sparc := sparc
UCLIBC_BASE       = $(PKGDIR)/../uclibc
UCLIBC_SYSDEPS    = $(UCLIBC_BASE)/lib/contrib/uclibc/libc/sysdeps/linux/$(UCLIBC_ARCH_$(ARCH))

PRIVATE_INCDIR    = $(PKGDIR) $(UCLIBC_SYSDEPS)

vpath %.S $(UCLIBC_SYSDEPS)

INSTALL_TARGET	= $(LD_SCRIPTS) crti.o crtn.o crt1.o crti.s.o crtn.s.o crt1.s.o

include $(L4DIR)/mk/lib.mk

$(eval $(call BID_MAKE_RULE_template, crti_s.o, crti.S,AS))

crti.o: crti_s.o kip_addr.o
	@$(GEN_MESSAGE)
	$(VERBOSE)$(LD) -r -o $@ $^

%.ld: $(PKGDIR)/ARCH-$(ARCH)/%.ld $(SRC_DIR)/Makefile $(SRC_DIR)/generic.h
	@$(GEN_MESSAGE)
	$(VERBOSE)[ "$(LDVERSION)" -lt "218" ] && \
	  CPP_ARGS="-DMAXPAGESIZE=$(MAXPAGESIZE_$(ARCH)) -DCOMMONPAGESIZE=$(COMMONPAGESIZE_$(ARCH)) -DCONSTANT(x)=x"; \
	  $(CPP) -nostdinc -include $(SRC_DIR)/generic.h \
	      -Wno-trigraphs -U$(UCLIBC_ARCH_$(ARCH)) $(CFLAGS) -P $$CPP_ARGS $< $@
