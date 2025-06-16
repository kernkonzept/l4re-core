# Directories
DIRS-all     := libc/string libc/stdlib libc/stdio libc/unistd libc/signal \
                libc/misc libc/sysdeps/linux

DIRS-minimal := $(DIRS-all) libc/stdlib/malloc
DIRS-full    := $(DIRS-all) libc/inet libc/pwd_grp libiconv libc/termios     \
                libc/stdlib/malloc$(if $(CONFIG_BID_STATIC_HEAP),,-standard) \
                $(if $(BID_VARIANT_FLAG_NOFPU),,libm) libcrypt libuargp

DIRS         := $(DIRS-$(LIBC_BUILD_MODE))

# Sub Modules
SUB_MODULES-all     := large_file

SUB_MODULES-minimal := $(SUB_MODULES-all)
SUB_MODULES-full    := $(SUB_MODULES-all) wchar locale wchar_locale \
                       $(if $(BID_VARIANT_FLAG_NOFPU),,fp)

SUB_MODULES := $(SUB_MODULES-$(LIBC_BUILD_MODE))

