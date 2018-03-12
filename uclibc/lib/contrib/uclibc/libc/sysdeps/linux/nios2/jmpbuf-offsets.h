/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

#define JB_REGS		0
#define JB_PC		32
#define JB_SP		36
#define JB_FP		40
#define JB_GP		44
#define JB_FPREGS 	48

#ifdef __UCLIBC_HAS_FPU__
# define JB_SIZE 304
#else
# define JB_SIZE 48
#endif
