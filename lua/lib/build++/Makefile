PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

LUA_VERSION_SUFFIX :=
INSTALL_TARGET     := liblua$(LUA_VERSION_SUFFIX)++.a
PC_LIBS            := -llua$(LUA_VERSION_SUFFIX)++
CONTRIB_INCDIR     := lua$(LUA_VERSION_SUFFIX)
REQUIRES_LIBS      := l4re_c-util
PC_FILENAME        := lua$(LUA_VERSION_SUFFIX)++
OPTS               :=

all::

include $(L4DIR)/mk/lib.mk

export VERBOSE

DEFS = -DARCH_$(ARCH) $(OPTS)

liblua.a: $(SRC_DIR)/Makefile $(SRC_DIR)/Makefile.lua FORCE
	$(MAKE) -f $(SRC_DIR)/Makefile.lua SRC_DIR=$(SRC_DIR) \
	  L4_DEFINES="$(DEFS)" GENERAL_D_LOC=$(GENERAL_D_LOC) \
	  L4_INCLUDES="$(LIBCINCDIR) $(addprefix -I, $(L4INCDIR))" \
	  PLAT=l4re CXXFLAGS_L4_GENERIC="$(CXXFLAGS_L4_GENERIC)" \
	  CC="$(CXX)" AR="$(AR) rcu" RANLIB="$(RANLIB)" liblua.a

ifneq ($(strip $(INSTALL_TARGET)),liblua.a)
$(INSTALL_TARGET): liblua.a
	$(LN) -sf $< $@
endif

clean::
	$(MAKE) -f $(SRC_DIR)/Makefile.lua SRC_DIR=$(SRC_DIR) clean


all:: $(PC_FILES)
