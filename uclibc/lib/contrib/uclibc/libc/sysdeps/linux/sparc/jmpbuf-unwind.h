/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
#include <setjmp.h>
#include <jmpbuf-offsets.h>

#if __WORDSIZE == 64

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((unsigned long int) (address) < (jmpbuf)->uc_mcontext.mc_fp)

#else

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((int) (address) < (jmpbuf)[JB_SP])

#endif

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#if defined(__arch64__)
#include "sparc64/jmpbuf-unwind.h"
#else
#include "sparc32/jmpbuf-unwind.h"
#endif
#endif
