PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

include $(L4DIR)/mk/Makeconf

TARGET      = libc_support_misc.ofl libc_support_misc.p.a
PC_FILENAME = libc_support_misc
PC_LIBS        = %{-link-libc:%{shared:--whole-archive -lc_support_misc.p --no-whole-archive;:libc_support_misc.ofl}}
PC_LIBS_PIC    =
SRC_C       = daemon.c \
              exec.c \
              fork.c \
              getloadavg.c \
              getpass.c \
              getpid.c \
              getrusage.c \
              kill.c \
              limit.c \
              pathconf.c \
              pipe.c \
              posix_fadvise.c \
              prctl.c \
              sched.c \
              secure_getenv.c \
              shadow.c \
              shm.c \
              spawn.c \
              sysconf.c \
              system.c \
              termios.c \
              timer.c \
              times.c \
              uidgid.c \
              umask.c \
              uname.c \
              wait.c \
              wait3.c \
              waitpid.c

SRC_C-$(CONFIG_L4_LIBC_UCLIBC) += lockf.c

include $(L4DIR)/mk/lib.mk

CPPFLAGS-$(CONFIG_L4_LIBC_UCLIBC) += -DNOT_IN_libc -include libc-symbols.h
CPPFLAGS-$(CONFIG_L4_LIBC_MUSL)   += -D_GNU_SOURCE
