# vim:set ft=make:

vpath %.c $(SRC_DIR)/../contrib/src $(SRC_DIR)/../l4
vpath %.h $(SRC_DIR)/../contrib/src

%.o: %.c $(GENERAL_D_LOC) $(SRC_DIR)/Makefile.lua
	@echo " CXX $@"
	$(VERBOSE)$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

include $(SRC_DIR)/../contrib/src/Makefile
MYCFLAGS=-I$(SRC_DIR)/../contrib/src -DLUA_USE_L4RE $(L4_DEFINES) $(CXXFLAGS_L4_GENERIC) $(L4_INCLUDES) #-Weverything
