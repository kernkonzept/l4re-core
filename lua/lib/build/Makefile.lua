# vim:set ft=make:

vpath %.c $(SRC_DIR)/../contrib/src
vpath %.h $(SRC_DIR)/../contrib/src

%.o: %.c $(GENERAL_D_LOC) $(SRC_DIR)/Makefile.lua
	@echo " CC $@"
	$(VERBOSE)$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

include $(SRC_DIR)/../contrib/src/Makefile
MYCFLAGS=-I$(SRC_DIR)/../contrib/src -DLUA_USE_L4RE $(L4_DEFINES) $(CFLAGS_L4_GENERIC) $(L4_INCLUDES)
