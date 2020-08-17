PKGDIR  ?= ../..
L4DIR   ?= $(PKGDIR)/../../..

# ?= for inclusion from build++
TARGET        ?= liblua.a liblua.so
PC_FILENAME   ?= lua
CONTRIB_DIR    = $(SRC_DIR)/../contrib/src
CONTRIB_INCDIR = lua
SRC_C         := lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c \
                 llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c \
                 lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c \
                 lbaselib.c lcorolib.c ldblib.c liolib.c lmathlib.c loadlib.c \
                 loslib.c lstrlib.c ltablib.c lutf8lib.c linit.c

vpath %.c $(CONTRIB_DIR)

include $(L4DIR)/mk/lib.mk

PKGNAME_DIRNAME = lua-c

WARNINGS := $(WARNINGS_MEDIUM)

# convenience helper
showluasrc:
	$(VERBOSE)(cat $(CONTRIB_DIR)/Makefile; echo show:; \
	  echo '$$(info SRC_C := $$(patsubst %.o,%.c,$$(BASE_O)))') \
	  | env -i $(MAKE) -f - show
