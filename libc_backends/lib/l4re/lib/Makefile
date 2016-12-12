PKGDIR ?= ../../..
L4DIR  ?= $(PKGDIR)/../../..

TARGET        = libc_be_l4re.a libc_be_l4re.so
PC_FILENAME   = libc_be_l4re
PC_EXTRA     := Link_Libs= %{static:-lc_be_l4re -l4re}
PC_LIBS      := -lc_be_l4re
REQUIRES_LIBS = l4re
SRC_C         = nanosleep.c sched_yield.c usleep.c \
                gettimeofday.c clock_gettime.c clock.c \
                clock_settime.c settimeofday.c time.c
CFLAGS        = -ffunction-sections

include $(L4DIR)/mk/lib.mk
