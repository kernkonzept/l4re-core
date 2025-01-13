PKGDIR ?= ../../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET        = libc_be_l4re.ofl libc_be_l4re.p.a
PC_FILENAME   = libc_be_l4re
PC_LIBS       = %{-link-libc:%{shared:--whole-archive -lc_be_l4re.p --no-whole-archive;:libc_be_l4re.ofl}}
PC_LIBS_PIC   =
SRC_C         = nanosleep.c sched_yield.c usleep.c \
                gettimeofday.c clock_gettime.c clock.c \
                clock_settime.c settimeofday.c time.c \
                clock_getres.c
CFLAGS        = -ffunction-sections

include $(L4DIR)/mk/lib.mk
