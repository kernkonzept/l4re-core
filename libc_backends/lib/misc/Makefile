PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

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
              lockf.c \
              pathconf.c \
              pipe.c \
              posix_fadvise.c \
              prctl.c \
              ptsname.c \
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
CFLAGS      = -DNOT_IN_libc -include libc-symbols.h

include $(L4DIR)/mk/lib.mk
