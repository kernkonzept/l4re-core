PKGDIR	 = .
L4DIR	?= $(PKGDIR)/../../..

TARGET := include include-gen
# include needs include-gen as prerequisite because it installs a header file
# that is symlinked by include-gen.
include: include-gen

include $(L4DIR)/mk/subdir.mk
