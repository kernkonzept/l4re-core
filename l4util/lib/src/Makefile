PKGDIR	?= ../..
L4DIR	?= $(PKGDIR)/../../..

TARGET                = libl4util.a libl4util.so
REQUIRES_LIBS         = l4sys
PC_EXTRA              = Link_Libs= %{static|static-pie:-ll4util}

ALL_SRC_C_only_x86    = $(addprefix ARCH-x86/, perform.c spin.c rdtsc.c)
ALL_SRC_C_only_amd64  = $(ALL_SRC_C_only_x86)
ALL_SRC_C_only_ppc32  = $(addprefix ARCH-ppc32/, rdtsc.c)
ALL_SRC_C_only_sparc  = ARCH-sparc/atomics.c
SRC_C                 = getopt2.c micros2l4to.c rand.c sleep.c \
                        base64.c kprintf.c kip.c keymap.c \
                        ARCH-$(ARCH)/backtrace.c thread.c \
                        $(ALL_SRC_C_only_$(ARCH)) parse_cmdline.c \
                        list_alloc.c
CXXFLAGS              = -DL4_NO_RTTI -fno-exceptions -fno-rtti

PRIVATE_INCDIR_ARCH-x86/perform.c = ARCH-x86

include $(L4DIR)/mk/lib.mk

ARCH-x86/perform.o ARCH-x86/perform.s.o: ARCH-x86/pmc_events.h
ARCH-x86/pmc_events.h: pmc_events.in
	@$(GEN_MESSAGE)
	$(VERBOSE)$(MKDIR) $(dir $@)
	$(VERBOSE)sort < $^ > $(^F).sorted || rm $(^F).sorted
	$(VERBOSE)echo "/* created automatically from $^, do not edit */" > $@
	$(VERBOSE)awk '/^[0-9a-fA-F][0-9a-fA-F].*/{print "{0x"toupper($$1)", \""$$2"\"},"}' < $(^F).sorted >> $@ || rm $@
	$(VERBOSE)rm $(^F).sorted

vpath pmc_events.in $(SRC_DIR)/ARCH-x86
