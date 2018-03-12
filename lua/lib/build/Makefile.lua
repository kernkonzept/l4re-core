# vim:set ft=make:

vpath %.c $(PKGDIR)/lib/contrib/src
vpath %.h $(PKGDIR)/lib/contrib/src

%.o: %.c $(GENERAL_D_LOC)
	$(CC) -c $(CPPFLAGS) $(CFLAGS)  $<

include $(PKGDIR)/lib/contrib/src/Makefile
MYCFLAGS=-I$(PKGDIR)/contrib/src -DLUA_USE_L4RE $(L4_DEFINES) $(CFLAGS_L4_GENERIC) $(L4_INCLUDES)
