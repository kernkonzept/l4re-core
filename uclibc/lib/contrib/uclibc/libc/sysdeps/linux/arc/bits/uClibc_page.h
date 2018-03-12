/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#ifndef _UCLIBC_PAGE_H
#define _UCLIBC_PAGE_H

/*
 * ARC700/linux supports 4k, 8k, 16k pages (build time).
 * We rely on the kernel exported header (aka uapi headers since 3.8)
 * for PAGE_SIZE and friends. This avoids hand-editing here when building
 * toolchain.
 *
 * Although uClibc determines page size dynamically, from kernel's auxv which
 * ARC Linux does pass, still the generic code needs a fall back
 *  _dl_pagesize = auxvt[AT_PAGESZ].a_un.a_val ? : PAGE_SIZE
 *
 */
#include <asm/page.h>

/* TBD: fix this with runtime value for a PAGE_SIZE agnostic uClibc */
#define MMAP2_PAGE_SHIFT PAGE_SHIFT

#endif /* _UCLIBC_PAGE_H */
