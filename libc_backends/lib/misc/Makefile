PKGDIR ?= ../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET      = libc_support_misc.a libc_support_misc.so
PC_FILENAME = libc_support_misc
PC_EXTRA    = Link_Libs= %{static|static-pie:-lc_support_misc}
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
              shm.c \
              sysconf.c \
              system.c \
              termios.c \
              timer.c \
              times.c \
              uidgid.c \
              umask.c \
              uname.c \
              wait3.c \
              waitpid.c

include $(L4DIR)/mk/lib.mk
