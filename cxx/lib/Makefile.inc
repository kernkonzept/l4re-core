# vim:set ft=make:

TARGET_BASENAME := $(TARGET)

TARGET_A 	:= $(TARGET_BASENAME).a
TARGET_E_A	:= $(TARGET_BASENAME)_e.a

PC_FILENAMES             := $(PC_FILENAME) $(PC_FILENAME)_e
PC_LIBS_$(PC_FILENAME)   := -l$(patsubst lib%,%,$(TARGET_BASENAME))
PC_LIBS_$(PC_FILENAME)_e := -l$(patsubst lib%,%,$(TARGET_BASENAME))_e

$(foreach src,$(SRC_CC),$(eval CXXFLAGS_$$(src) += -DL4_NO_RTTI -fno-exceptions -fno-rtti))

SRC_CC_$(TARGET_BASENAME).a     := $(SRC_CC)
SRC_CC_$(TARGET_BASENAME).p.a   := $(SRC_CC)
SRC_CC_$(TARGET_BASENAME)_e.a   := $(patsubst %.cc,%.e.cc,$(SRC_CC))
SRC_CC_$(TARGET_BASENAME)_e.p.a := $(patsubst %.cc,%.e.cc,$(SRC_CC))
SRC_CC 		      :=

PRIVATE_INCDIR	+= $(SRC_DIR)/

TARGET := $(TARGET_BASENAME).a $(TARGET_BASENAME).p.a \
          $(TARGET_BASENAME)_e.a $(TARGET_BASENAME)_e.p.a

all::

%.e.cc: %.cc
	$(VERBOSE)ln -s $< $@
