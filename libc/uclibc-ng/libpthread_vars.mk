CFLAGS         += -include $(CONTRIB_DIR)/include/libc-symbols.h
CXXFLAGS       += -include $(CONTRIB_DIR)/include/libc-symbols.h
CXXFLAGS       += -fno-exceptions
PRIVATE_INCDIR += $(CONTRIB_DIR)/ldso/include
