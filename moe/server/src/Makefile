PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../../..

TARGET          = moe
DEFAULT_RELOC   = 0x0140000
RELOC_PHYS      = y
SRC_CC          = main.cc page_alloc.cc dataspace.cc dataspace_cont.cc \
                  dataspace_anon.cc globals.cc \
                  alloc.cc boot_fs.cc dataspace_util.cc \
                  region.cc debug.cc malloc.cc quota.cc \
                  loader.cc loader_elf.cc exception.cc \
                  app_task.cc dataspace_noncont.cc pages.cc \
                  name_space.cc mem.cc log.cc sched_proxy.cc \
                  delete.cc vesa_fb.cc server_obj.cc \
                  dma_space.cc
SRC_S          := ARCH-$(ARCH)/crt0.S
MODE            = sigma0

REQUIRES_LIBS  := libkproxy libloader l4re-util libsigma0 \
                  cxx_io cxx_libc_io libc_be_minimal_log_io libsupc++_minimal
EXTRA_LIBS     := -ll4sys-direct
DEFINES        += -DL4_CXX_NO_EXCEPTION_BACKTRACE -DL4_MINIMAL_LIBC
LDFLAGS        += --entry=_real_start

include $(L4DIR)/mk/prog.mk
