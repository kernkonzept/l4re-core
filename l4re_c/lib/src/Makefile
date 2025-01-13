PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../../..

TARGET	      = lib4re-c.ofl lib4re-c.p.a
PC_LIBS       = %{-link-libc:%{shared:--whole-archive -l4re-c.p --no-whole-archive;:lib4re-c.ofl}}
PC_LIBS_PIC   =

SRC_CC := namespace.cc \
          rm.cc \
          mem_alloc.cc \
          log.cc \
          dataspace.cc \
          debug.cc \
          event.cc \
          event_buffer.cc \
          video.cc \
          view.cc \
          goos.cc \
          inhibitor.cc \
          dma_space.cc \
          parent.cc

CXXFLAGS += -DL4_NO_RTTI -fno-rtti -fno-exceptions

include $(L4DIR)/mk/lib.mk
