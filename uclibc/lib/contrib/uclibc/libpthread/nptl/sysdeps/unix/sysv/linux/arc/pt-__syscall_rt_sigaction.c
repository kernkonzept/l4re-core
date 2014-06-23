/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

/*
 * ARC syscall ABI only has __NR_rt_sigaction, thus vanilla sigaction does
 * some SA_RESTORER tricks before calling __syscall_rt_sigaction.
 * However including that file here causes a redefinition of __libc_sigaction
 * in static links involving pthreads
 */
//#include <../../../../../../../libc/sysdeps/linux/arc/sigaction.c>
