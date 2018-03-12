/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _SYS_STATFS_H
# error "Never include <bits/statfs.h> directly; use <sys/statfs.h> instead."
#endif

#include <endian.h>
#include <bits/align64bit.h>
#include <bits/types.h>


struct statfs
  {
    __U32_TYPE f_type;
    __U32_TYPE f_bsize;
#ifndef __USE_FILE_OFFSET64
# if __BYTE_ORDER == __LITTLE_ENDIAN
    __U32_TYPE f_blocks;
    __U32_TYPE __pad1;
    __U32_TYPE f_bfree;
    __U32_TYPE __pad2;
    __U32_TYPE f_bavail;
    __U32_TYPE __pad3;
    __U32_TYPE f_files;
    __U32_TYPE __pad4;
    __U32_TYPE f_ffree;
    __U32_TYPE __pad5;
# else
    __U32_TYPE __pad1;
    __U32_TYPE f_blocks;
    __U32_TYPE __pad2;
    __U32_TYPE f_bfree;
    __U32_TYPE __pad3;
    __U32_TYPE f_bavail;
    __U32_TYPE __pad4;
    __U32_TYPE f_files;
    __U32_TYPE __pad5;
    __U32_TYPE f_ffree;
# endif /* __LITTLE_ENDIAN */
#else
    __U64_TYPE f_blocks;
    __U64_TYPE f_bfree;
    __U64_TYPE f_bavail;
    __U64_TYPE f_files;
    __U64_TYPE f_ffree;
#endif /* __USE_FILE_OFFSET64 */
    __fsid_t f_fsid;
    __U32_TYPE f_namelen;
    __U32_TYPE f_frsize;
    __U32_TYPE f_flags;
    __U32_TYPE f_spare[4];
  } __ARCH_64BIT_ALIGNMENT__;

#ifdef __USE_LARGEFILE64
struct statfs64
  {
    __U32_TYPE f_type;
    __U32_TYPE f_bsize;
    __U64_TYPE f_blocks;
    __U64_TYPE f_bfree;
    __U64_TYPE f_bavail;
    __U64_TYPE f_files;
    __U64_TYPE f_ffree;
    __fsid_t f_fsid;
    __U32_TYPE f_namelen;
    __U32_TYPE f_frsize;
    __U32_TYPE f_flags;
    __U32_TYPE f_spare[4];
  };
#endif

/* Tell code we have these members.  */
#define _STATFS_F_NAMELEN
#define _STATFS_F_FRSIZE
