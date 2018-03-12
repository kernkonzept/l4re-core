PKGDIR?= ../..
L4DIR ?= $(PKGDIR)/../../..

TARGET	      = lib4re-c.a lib4re-c.so
REQUIRES_LIBS = l4re
SRC_CC := namespace.cc \
          rm.cc \
          mem_alloc.cc \
          io_stuff.cc \
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

include $(L4DIR)/mk/lib.mk
