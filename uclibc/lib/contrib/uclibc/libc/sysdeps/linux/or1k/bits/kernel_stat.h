#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

#if defined(__UCLIBC_USE_TIME64__)
#include "internal/time64_helpers.h"
#endif

struct kernel_stat {
	unsigned long	st_dev;		/* Device.  */
	unsigned long	st_ino;		/* File serial number.  */
	unsigned int	st_mode;	/* File mode.  */
	unsigned int	st_nlink;	/* Link count.  */
	unsigned int	st_uid;		/* User ID of the file's owner.  */
	unsigned int	st_gid;		/* Group ID of the file's group. */
	unsigned long	st_rdev;	/* Device number, if device.  */
	unsigned long	__pad1;
	long		st_size;	/* Size of file, in bytes.  */
	int		st_blksize;	/* Optimal block size for I/O.  */
	int		__pad2;
	long		st_blocks;	/* Number 512-byte blocks allocated. */
#if defined(__UCLIBC_USE_TIME64__)
	struct __ts32_struct __st_atim32;
	struct __ts32_struct __st_mtim32;
	struct __ts32_struct __st_ctim32;
#else
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
#endif
	unsigned int	__unused4;
	unsigned int	__unused5;
};

#define kernel_stat64	kernel_stat

#endif	/* _BITS_STAT_STRUCT_H */

