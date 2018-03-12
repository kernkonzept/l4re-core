# vim:set ft=make:

TARGET_BASENAME := $(TARGET)

TARGET_A 	:= $(TARGET_BASENAME).a
TARGET_E_A	:= $(TARGET_BASENAME)_e.a

PC_FILENAMES             := $(PC_FILENAME) $(PC_FILENAME)_e
PC_LIBS_$(PC_FILENAME)   := -l$(patsubst lib%,%,$(TARGET_BASENAME))
PC_LIBS_$(PC_FILENAME)_e := -l$(patsubst lib%,%,$(TARGET_BASENAME))_e

CXXFLAGS_F_A	      := $(patsubst %.cc,%.cxxflags,$(SRC_CC))
CXXFLAGS_F_E_A	      := $(patsubst %.cc,%.e.cxxflags,$(SRC_CC))

SRC_CC_$(TARGET_BASENAME).a     := $(SRC_CC)
SRC_CC_$(TARGET_BASENAME).p.a   := $(SRC_CC)
SRC_CC_$(TARGET_BASENAME)_e.a   := $(patsubst %.cc,%.e.cc,$(SRC_CC))
SRC_CC_$(TARGET_BASENAME)_e.p.a := $(patsubst %.cc,%.e.cc,$(SRC_CC))
SRC_CC 		      :=

PRIVATE_INCDIR	+= $(SRC_DIR)/

TARGET := $(TARGET_BASENAME).a $(TARGET_BASENAME).p.a \
          $(TARGET_BASENAME)_e.a $(TARGET_BASENAME)_e.p.a

ifneq ($(MAKECMDGOALS),)
  MAKEGOALS	:= $(MAKECMDGOALS)
else
  MAKEGOALS	:= all
endif

ifeq ($(findstring OBJ,$(shell pwd)),OBJ)
ifneq ($(filter-out clean cleanall scrub,$(MAKEGOALS)),)
  -include $(CXXFLAGS_F_A) $(CXXFLAGS_F_E_A)
endif
endif

all::

$(CXXFLAGS_F_A): %.cxxflags: %.cc $(CXX_PKG_DIR)/lib/Makefile.inc
	$(VERBOSE)echo "CXXFLAGS_$(notdir $<) := -DL4_NO_RTTI -Wall -fno-exceptions -fno-rtti" > $@

$(CXXFLAGS_F_E_A): %.cxxflags: %.cc $(CXX_PKG_DIR)/lib/Makefile.inc
	$(VERBOSE)echo "CXXFLAGS_$(notdir $<) := -Wall" > $@

%.e.cc: %.cc
	$(VERBOSE)ln -s $< $@
