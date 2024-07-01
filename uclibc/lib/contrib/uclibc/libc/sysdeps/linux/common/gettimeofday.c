/*
 * gettimeofday() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/time.h>

#ifdef __VDSO_SUPPORT__
#include "ldso.h"
#endif


#ifdef __VDSO_SUPPORT__
typedef int (*gettimeofday_func)(struct timeval * tv, __timezone_ptr_t tz);
#endif

int gettimeofday(struct timeval * tv, __timezone_ptr_t tz) {

    #ifdef __VDSO_SUPPORT__
        if ( _dl__vdso_gettimeofday != 0 ){
            gettimeofday_func func= _dl__vdso_gettimeofday;
            return func( tv, tz );

        }else{
            _syscall2_body(int, gettimeofday, struct timeval *, tv, __timezone_ptr_t, tz)
        }
    #else
        _syscall2_body(int, gettimeofday, struct timeval *, tv, __timezone_ptr_t, tz)
    #endif
}


libc_hidden_def(gettimeofday)
