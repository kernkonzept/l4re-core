PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../../..

TARGET		             = libl4sys.ofl      libl4sys.p.a     \
                               libl4sys-direct.a \
                               libl4sys-l4x.a    libl4sys-l4x.so
PC_FILENAMES                 = l4sys l4sys-direct l4sys-l4x
PC_LIBS_l4sys                = %{-link-libc:%{shared:--whole-archive -ll4sys.p --no-whole-archive;:libl4sys.ofl}}
PC_LIBS_l4sys-direct         = -ll4sys-direct
PC_EXTRA_l4sys-direct        = Link_Libs= %{static|static-pie:-ll4sys-direct}
PC_LIBS_l4sys-l4x            = -ll4sys-l4x
PC_EXTRA_l4sys-l4x           = Link_Libs= %{static|static-pie:-ll4sys-l4x}

PC_LIBS_PIC =

CXXFLAGS                    += -fno-rtti -fno-exceptions
DEFINES                      = -DL4_KIP_ADDR=$(L4_KIP_ADDR)

SRC_CC                       = kobject.cc
SRC_S_libl4sys-arm          += ARCH-arm/atomic_ops_s.S
SRC_C_libl4sys-ppc32        += ARCH-ppc32/atomic_ops.c
SRC_S_libl4sys-sparc        += ARCH-sparc/atomic_ops.S

SRC_S_libl4sys.ofl-arm       = ARCH-arm/syscall.S
SRC_S_libl4sys.ofl-arm64     = ARCH-arm64/syscall.S
SRC_S_libl4sys.p.a-arm       = ARCH-arm/syscall.S
SRC_S_libl4sys.p.a-arm64     = ARCH-arm64/syscall.S

SRC_S_libl4sys-direct.a-arm  = ARCH-arm/syscall.S ARCH-arm/atomic_ops_s.S
SRC_S_libl4sys-direct.a-arm64 = ARCH-arm64/syscall.S
SRC_S_libl4sys-direct.a-x86  = ARCH-x86/syscalls_direct.S
SRC_S_libl4sys-direct.a      = kip.S $(SRC_S_libl4sys-direct.a-$(ARCH))

SRC_C_libl4sys.ofl           = utcb.c errtostr.c $(SRC_C_libl4sys-$(ARCH))
SRC_C_libl4sys.p.a           = utcb.c errtostr.c $(SRC_C_libl4sys-$(ARCH))
SRC_S_libl4sys.ofl           = kip.S $(SRC_S_libl4sys.ofl-$(ARCH)) \
                               $(SRC_S_libl4sys-$(ARCH))
SRC_S_libl4sys.p.a           = $(SRC_S_libl4sys.p.a-$(ARCH)) \
                               $(SRC_S_libl4sys-$(ARCH))

SRC_C_libl4sys-l4x.a         = utcb.c errtostr.c $(SRC_C_libl4sys-$(ARCH))
SRC_C_libl4sys-l4x.so        = utcb.c errtostr.c $(SRC_C_libl4sys-$(ARCH))
SRC_S_libl4sys-l4x.a         = kip.S $(SRC_S_libl4sys-$(ARCH))
SRC_S_libl4sys-l4x.so        = $(SRC_S_libl4sys-$(ARCH))

SRC_C_l4lx-arm               = utcb.c
SRC_C_l4lx-amd64             = utcb.c
SRC_S_l4lx-arm               = ARCH-arm/atomic_ops_s.S
SRC_C_l4lx-mips              = utcb.c
SRC_C_l4lx-ppc32             = utcb.c ARCH-ppc32/atomic_ops.c
SRC_C_l4lx-riscv             = utcb.c
SRC_C_l4lx-x86               = ARCH-x86/utcb-l4x.c
SRC_C_libl4sys-l4x.a         = errtostr.c $(SRC_C_l4lx-$(ARCH))
SRC_S_libl4sys-l4x.a         = $(SRC_S_l4lx-$(ARCH))

COMPILER_RT                  = compiler-rt-pure

include $(L4DIR)/mk/lib.mk
