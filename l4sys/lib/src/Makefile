PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		             = libl4sys.a        libl4sys.so        \
                               libl4sys-direct.a libl4sys-direct.so \
                               libl4sys-l4x.a
PC_FILENAMES                 = l4sys l4sys-direct l4sys-l4x
PC_LIBS_l4sys                = -ll4sys
PC_EXTRA_l4sys               = Link_Libs= %{static|static-pie:-ll4sys}
PC_LIBS_l4sys-direct         = -ll4sys-direct
PC_EXTRA_l4sys-direct        = Link_Libs= %{static|static-pie:-ll4sys-direct}
PC_LIBS_l4sys-l4x            = -ll4sys-l4x
PC_EXTRA_l4sys-l4x           = Link_Libs= %{static|static-pie:-ll4sys-l4x}

CXXFLAGS                    += -DL4_NO_RTTI -fno-rtti -fno-exceptions
DEFINES                      = -D__L4_KIP_ADDR__=$(L4_KIP_ADDR)

SRC_S_libl4sys-arm          += ARCH-arm/atomic_ops_s.S
SRC_S_libl4sys-arm64        += ARCH-arm64/ipc.S
SRC_C_libl4sys-ppc32        += ARCH-ppc32/atomic_ops.c
SRC_S_libl4sys-sparc        += ARCH-sparc/atomic_ops.S

SRC_S_libl4sys-direct.so     =
SRC_S_libl4sys-direct.a-x86  = syscalls_direct.S
SRC_S_libl4sys-direct.a      = $(SRC_S_libl4sys-direct.a-$(ARCH))
vpath syscalls_direct.S $(SRC_DIR)/ARCH-$(ARCH)

SRC_C_libl4sys.a             = utcb.c errtostr.c kip.c $(SRC_C_libl4sys-$(ARCH))
SRC_C_libl4sys.so            = utcb.c errtostr.c kip.c $(SRC_C_libl4sys-$(ARCH))
SRC_CC_libl4sys.a            = kobject.cc
SRC_CC_libl4sys.so           = kobject.cc
SRC_S_libl4sys.a             = $(SRC_S_libl4sys-$(ARCH))
SRC_S_libl4sys.so            = $(SRC_S_libl4sys-$(ARCH))

# Build libl4sys-l4x.a for L4Linux. Built with the same sources libl4sys.a but
# with different flags to match the ABI of L4Linux.

SRC_C_libl4sys-l4x.a         = $(SRC_C_libl4sys.a)
SRC_CC_libl4sys-l4x.a        = $(SRC_CC_libl4sys.a)
SRC_S_libl4sys-l4x.a         = $(SRC_S_libl4sys.a)

CFLAGS_l4x-arm               = -msoft-float
CFLAGS_l4x-arm64             = -mabi=lp64

include $(L4DIR)/mk/lib.mk

$(eval $(call BID_MAKE_RULE_template, %.l4x.o, %.c,C,$(CFLAGS_l4x-$(ARCH))))
$(eval $(call BID_MAKE_RULE_template, %.l4x.o, %.cc,CXX,$(CFLAGS_l4x-$(ARCH))))
$(eval $(call BID_MAKE_RULE_template, %.l4x.o, %.S,AS,$(CFLAGS_l4x-$(ARCH))))

OBJS_libl4sys-l4x.a =
$(call GENERATE_PER_TARGET_RULES,libl4sys-l4x.a,.l4x)

GCCLIB    = $(OBJ_BASE)/lib/$(ARCH)_$(CPU)/libgcc.a
GCCLIB_SO = $(OBJ_BASE)/lib/$(ARCH)_$(CPU)/libgcc.a
