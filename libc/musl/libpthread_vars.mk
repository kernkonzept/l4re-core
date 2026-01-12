CFLAGS         += -include $(CONTRIB_DIR)/src/internal/libc.h
CXXFLAGS       += -include $(CONTRIB_DIR)/src/internal/libc.h
DEFINES        += -Dhidden='__attribute__((__visibility__("hidden")))'
CXXFLAGS       += -fno-exceptions
PRIVATE_INCDIR += $(CONTRIB_DIR)/arch/$(LIBC_ARCH)

SRC_C_libc_pthread += impl-musl.c
