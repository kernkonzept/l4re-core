PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../..

# CRT0 lib for statically linked binaries
SRC_C_libcrt0.a          := construction.c support.c 
SRC_S_libcrt0.a          := crt0_x.S

# CRT0 lib for dynamically linked binaries
SRC_C_libcrt0_s.a        := construction.c
SRC_S_libcrt0_s.a        := crt0_x.S

# Stand alone lib for initializing the EH frames in statically
# linked binaries (usually this in n_crt0.S
SRC_CC_libcrt0_eh.o.a := init_eh_frame.cc

TARGET		= libcrt0.a libcrt0_s.a libcrt0_eh.o.a
MYOBJS		= crt0.o
SYSTEMS		= $(SYSTEMS_PLAIN)

# find the arch dependent things
vpath %.S  $(PKGDIR)/lib/src/ARCH-$(ARCH)

INSTALL_TARGET	= $(TARGET) $(MYOBJS)
KEEP_ON_CLEAN	= $(MYOBJS)

include $(L4DIR)/mk/lib.mk

crt0.o: crt0_x.o init_eh_frame.o
	@$(LINK_MESSAGE)
	$(VERBOSE)$(LD) $(filter-out -gc-sections,$(LDFLAGS)) -r $(filter %.o,$^) -o $@

crt0.s.o: support.s.o .general.d
	@$(LINK_MESSAGE)
	$(VERBOSE)$(LD) $(LDFLAGS) -r $(filter %.o,$^) -o $@
