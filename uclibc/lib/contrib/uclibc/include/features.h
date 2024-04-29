/* Copyright (C) 1991-1993,1995-2006,2007,2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef	_FEATURES_H
#define	_FEATURES_H	1

/* These are defined by the user (or the compiler)
   to specify the desired environment:

   __STRICT_ANSI__	ISO Standard C.
   _ISOC99_SOURCE	Extensions to ISO C89 from ISO C99.
   _POSIX_SOURCE	IEEE Std 1003.1.
   _POSIX_C_SOURCE	If ==1, like _POSIX_SOURCE; if >=2 add IEEE Std 1003.2;
			if >=199309L, add IEEE Std 1003.1b-1993;
			if >=199506L, add IEEE Std 1003.1c-1995;
			if >=200112L, all of IEEE 1003.1-2004
			if >=200809L, all of IEEE 1003.1-2008
   _XOPEN_SOURCE	Includes POSIX and XPG things.  Set to 500 if
			Single Unix conformance is wanted, to 600 for the
			sixth revision, to 700 for the seventh revision.
   _XOPEN_SOURCE_EXTENDED XPG things and X/Open Unix extensions.
   _LARGEFILE_SOURCE	Some more functions for correct standard I/O.
   _LARGEFILE64_SOURCE	Additional functionality from LFS for large files.
   _FILE_OFFSET_BITS=N	Select default filesystem interface.
   _BSD_SOURCE		ISO C, POSIX, and 4.3BSD things.
   _SVID_SOURCE		ISO C, POSIX, and SVID things.
   _ATFILE_SOURCE	Additional *at interfaces.
   _GNU_SOURCE		All of the above, plus GNU extensions.
   _DEFAULT_SOURCE	Equivalent to defining _BSD_SOURCE and _SVID_SOURCE,
			as well as _POSIX_C_SOURCE=200809L.
   _REENTRANT		Select additionally reentrant object.
   _THREAD_SAFE		Same as _REENTRANT, often used by other systems.
   _FORTIFY_SOURCE	If set to numeric value > 0 additional security
			measures are defined, according to level.

   The `-ansi' switch to the GNU C compiler defines __STRICT_ANSI__.
   If none of these are defined, the default is to have _SVID_SOURCE,
   _BSD_SOURCE, and _POSIX_SOURCE set to one and _POSIX_C_SOURCE set to
   200112L.  If more than one of these are defined, they accumulate.
   For example __STRICT_ANSI__, _POSIX_SOURCE and _POSIX_C_SOURCE
   together give you ISO C, 1003.1, and 1003.2, but nothing else.

   These are defined by this file and are used by the
   header files to decide what to declare or define:

   __USE_ISOC11		Define ISO C11 things.
   __USE_ISOC99		Define ISO C99 things.
   __USE_ISOC95		Define ISO C90 AMD1 (C95) things.
   __USE_POSIX		Define IEEE Std 1003.1 things.
   __USE_POSIX2		Define IEEE Std 1003.2 things.
   __USE_POSIX199309	Define IEEE Std 1003.1, and .1b things.
   __USE_POSIX199506	Define IEEE Std 1003.1, .1b, .1c and .1i things.
   __USE_XOPEN		Define XPG things.
   __USE_XOPEN_EXTENDED	Define X/Open Unix things.
   __USE_UNIX98		Define Single Unix V2 things.
   __USE_XOPEN2K        Define XPG6 things.
   __USE_XOPEN2K8       Define XPG7 things.
   __USE_LARGEFILE	Define correct standard I/O things.
   __USE_LARGEFILE64	Define LFS things with separate names.
   __USE_FILE_OFFSET64	Define 64bit interface as default.
   __USE_BSD		Define 4.3BSD things.
   __USE_SVID		Define SVID things.
   __USE_MISC		Define things common to BSD and System V Unix.
   __USE_ATFILE		Define *at interfaces and AT_* constants for them.
   __USE_GNU		Define GNU extensions.
   __USE_REENTRANT	Define reentrant/thread-safe *_r functions.
   __USE_FORTIFY_LEVEL	Additional security measures used, according to level.

   The macros `__GNU_LIBRARY__', `__GLIBC__', and `__GLIBC_MINOR__' are
   defined by this file unconditionally.  `__GNU_LIBRARY__' is provided
   only for compatibility.  All new code should use the other symbols
   to test for features.

   All macros listed above as possibly being defined by this file are
   explicitly undefined if they are not explicitly defined.
   Feature-test macros that are not defined by the user or compiler
   but are implied by the other feature-test macros defined (or by the
   lack of any definitions) are defined by the file.  */


/* Undefine everything, so we get a clean slate.  */
#undef	__USE_ISOC11
#undef	__USE_ISOC99
#undef	__USE_ISOC95
#undef	__USE_POSIX
#undef	__USE_POSIX2
#undef	__USE_POSIX199309
#undef	__USE_POSIX199506
#undef	__USE_XOPEN
#undef	__USE_XOPEN_EXTENDED
#undef	__USE_UNIX98
#undef	__USE_XOPEN2K
#undef	__USE_XOPEN2K8
#undef	__USE_LARGEFILE
#undef	__USE_LARGEFILE64
#undef	__USE_FILE_OFFSET64
#undef	__USE_BSD
#undef	__USE_SVID
#undef	__USE_MISC
#undef	__USE_ATFILE
#undef	__USE_GNU
#undef	__USE_REENTRANT
#undef	__USE_FORTIFY_LEVEL
#undef	__KERNEL_STRICT_NAMES

/* Suppress kernel-name space pollution unless user expressedly asks
   for it.  */
#ifndef _LOOSE_KERNEL_NAMES
# define __KERNEL_STRICT_NAMES
#endif

/* Always use ISO C things.  */
#define	__USE_ANSI	1

/* Convenience macros to test the versions of glibc and gcc.
   Use them like this:
   #if __GNUC_PREREQ (2,8)
   ... code requiring gcc 2.8 or later ...
   #endif
   Note - they won't work for gcc1 or glibc1, since the _MINOR macros
   were not defined then.  */
#if defined __GNUC__ && defined __GNUC_MINOR__
# define __GNUC_PREREQ(maj, min) \
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define __GNUC_PREREQ(maj, min) 0
#endif

/* Convenience macro to test the version of clang.
   Use like this:
   #if __CLANG_PREREQ(3,2)
   ... code requiring clang 3.2 or later ...
   #endif */
#if defined __clang__
# define __CLANG_PREREQ(maj, min) \
	((__clang_major__ << 16) + __clang_minor__ >= ((maj) << 16) + (min))
#else
# define __CLANG_PREREQ(maj, min) 0
#endif

/* Whether to use feature set F.  */
#define __GLIBC_USE(F)  __GLIBC_USE_ ## F

/* _DEFAULT_SOURCE is equivalent to defining _BSD_SOURCE, _SVID_SOURCE
 * and _POSIX_C_SOURCE=200809L and vice versa. */
#if defined _DEFAULT_SOURCE || defined _BSD_SOURCE || defined _SVID_SOURCE
# undef _DEFAULT_SOURCE
# define _DEFAULT_SOURCE 1
# undef  _BSD_SOURCE
# define _BSD_SOURCE	1
# undef  _SVID_SOURCE
# define _SVID_SOURCE	1
# if _POSIX_C_SOURCE < 200809L
#  undef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200809L
# endif
#endif

/* If _GNU_SOURCE was defined by the user, turn on all the other features.  */
#ifdef _GNU_SOURCE
# undef  _ISOC99_SOURCE
# define _ISOC99_SOURCE	1
# undef  _ISOC11_SOURCE
# define _ISOC11_SOURCE	1
# undef  _POSIX_SOURCE
# define _POSIX_SOURCE	1
# undef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE	200809L
# undef  _XOPEN_SOURCE
# define _XOPEN_SOURCE	700
# undef  _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED	1
# undef	 _LARGEFILE64_SOURCE
# define _LARGEFILE64_SOURCE	1
# undef  _BSD_SOURCE
# define _BSD_SOURCE	1
# undef  _SVID_SOURCE
# define _SVID_SOURCE	1
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE	1
#endif

/* This macro indicates that the installed library is uClibc.  Use
 * __UCLIBC_MAJOR__ and __UCLIBC_MINOR__ to test for the features in
 * specific releases.  */
#define	__UCLIBC__		1

#ifdef __UCLIBC__
/* Load up the current set of uClibc supported features along
 * with the current uClibc major and minor version numbers.
 * For uClibc release 0.9.26, these numbers would be:
 *	#define	__UCLIBC_MAJOR__	0
 *	#define	__UCLIBC_MINOR__	9
 *	#define	__UCLIBC_SUBLEVEL__	26
 */
# define __need_uClibc_config_h
# include <bits/uClibc_config.h>
# undef __need_uClibc_config_h

/* For uClibc, always optimize for size -- this should disable
 * a lot of expensive inlining...
 * TODO: this is wrong! __OPTIMIZE_SIZE__ is an indicator of
 * gcc -Os compile. We should not mess with compiler inlines.
 * We should instead disable __USE_EXTERN_INLINES unconditionally,
 * or maybe actually audit and test uclibc to work correctly
 * with __USE_EXTERN_INLINES on.
 */
# define __OPTIMIZE_SIZE__   1

/* disable unsupported features */
# undef __LDBL_COMPAT

/* no support for FORTIFY */
# undef _FORTIFY_SOURCE

# ifndef __UCLIBC_HAS_THREADS__
#  if defined _REENTRANT || defined _THREAD_SAFE
#   warning requested reentrant code, but thread support was disabled
#   undef _REENTRANT
#   undef _THREAD_SAFE
#  endif
# endif

#endif /* __UCLIBC__ */

/* If nothing (other than _GNU_SOURCE) is defined,
   define _BSD_SOURCE and _SVID_SOURCE.  */
#if (!defined __STRICT_ANSI__ && !defined _ISOC99_SOURCE && \
     !defined _POSIX_SOURCE && !defined _POSIX_C_SOURCE && \
     !defined _XOPEN_SOURCE && !defined _XOPEN_SOURCE_EXTENDED && \
     !defined _BSD_SOURCE && !defined _SVID_SOURCE)
# define _BSD_SOURCE	1
# define _SVID_SOURCE	1
#endif

/* This is to enable the ISO C11 extension.  */
#if (defined _ISOC11_SOURCE \
     || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L))
# define __USE_ISOC11	1
#endif

/* This is to enable the ISO C99 extension.  */
#if (defined _ISOC99_SOURCE || defined _ISOC11_SOURCE \
     || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L))
# define __USE_ISOC99	1
#endif

/* This is to enable the ISO C90 Amendment 1:1995 extension.  */
#if (defined _ISOC99_SOURCE || defined _ISOC9X_SOURCE \
     || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199409L))
# define __USE_ISOC95	1
#endif

/* If none of the ANSI/POSIX macros are defined, use POSIX.1 and POSIX.2
   (and IEEE Std 1003.1b-1993 unless _XOPEN_SOURCE is defined).  */
#if ((!defined __STRICT_ANSI__ || (_XOPEN_SOURCE - 0) >= 500) && \
     !defined _POSIX_SOURCE && !defined _POSIX_C_SOURCE)
# define _POSIX_SOURCE	1
# if defined _XOPEN_SOURCE && (_XOPEN_SOURCE - 0) < 500
#  define _POSIX_C_SOURCE	2
# elif defined _XOPEN_SOURCE && (_XOPEN_SOURCE - 0) < 600
#  define _POSIX_C_SOURCE	199506L
# elif defined _XOPEN_SOURCE && (_XOPEN_SOURCE - 0) < 700
#  define _POSIX_C_SOURCE	200112L
# else
#  define _POSIX_C_SOURCE	200809L
# endif
# define __USE_POSIX_IMPLICITLY	1
#endif

#if defined _POSIX_SOURCE || _POSIX_C_SOURCE >= 1 || defined _XOPEN_SOURCE
# define __USE_POSIX	1
#endif

#if defined _POSIX_C_SOURCE && _POSIX_C_SOURCE >= 2 || defined _XOPEN_SOURCE
# define __USE_POSIX2	1
#endif

#if (_POSIX_C_SOURCE - 0) >= 199309L
# define __USE_POSIX199309	1
#endif

#if (_POSIX_C_SOURCE - 0) >= 199506L
# define __USE_POSIX199506	1
#endif

#if (_POSIX_C_SOURCE - 0) >= 200112L
# define __USE_XOPEN2K		1
# undef __USE_ISOC99
# define __USE_ISOC99		1
#endif

#if (_POSIX_C_SOURCE - 0) >= 200809L
# define __USE_XOPEN2K8		1
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE	1
#endif

#ifdef	_XOPEN_SOURCE
# define __USE_XOPEN	1
# if (_XOPEN_SOURCE - 0) >= 500
#  define __USE_XOPEN_EXTENDED	1
#  define __USE_UNIX98	1
#  undef _LARGEFILE_SOURCE
#  define _LARGEFILE_SOURCE	1
#  if (_XOPEN_SOURCE - 0) >= 600
#   if (_XOPEN_SOURCE - 0) >= 700
#    define __USE_XOPEN2K8	1
#   endif
#   define __USE_XOPEN2K	1
#   undef __USE_ISOC99
#   define __USE_ISOC99		1
#  endif
# else
#  ifdef _XOPEN_SOURCE_EXTENDED
#   define __USE_XOPEN_EXTENDED	1
#  endif
# endif
#endif

#ifdef _LARGEFILE_SOURCE
# define __USE_LARGEFILE	1
#endif

#ifdef _LARGEFILE64_SOURCE
# define __USE_LARGEFILE64	1
#endif

#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64
# define __USE_FILE_OFFSET64	1
#endif

#if defined _BSD_SOURCE || defined _SVID_SOURCE
# define __USE_MISC	1
#endif

#ifdef	_BSD_SOURCE
# define __USE_BSD	1
#endif

#ifdef	_SVID_SOURCE
# define __USE_SVID	1
#endif

#ifdef	_ATFILE_SOURCE
# define __USE_ATFILE	1
#endif

#ifdef	_GNU_SOURCE
# define __USE_GNU	1
#endif

#if defined _REENTRANT || defined _THREAD_SAFE
# define __USE_REENTRANT	1
#endif

/* We do support the IEC 559 math functionality, real and complex.  */
#ifdef __UCLIBC_HAS_FLOATS__
#define __STDC_IEC_559__		1
#define __STDC_IEC_559_COMPLEX__	1
#endif

#ifdef __UCLIBC_HAS_WCHAR__
/* wchar_t uses ISO 10646-1 (2nd ed., published 2000-09-15) / Unicode 3.1.  */
#define __STDC_ISO_10646__		200009L
#endif

/*  There is an unwholesomely huge amount of code out there that depends on the
 *  presence of GNU libc header files.  We have GNU libc header files.  So here
 *  we commit a horrible sin.  At this point, we _lie_ and claim to be GNU libc
 *  to make things like /usr/include/linux/socket.h and lots of apps work as
 *  their developers intended.  This is IMHO, pardonable, since these defines
 *  are not really intended to check for the presence of a particular library,
 *  but rather are used to define an _interface_.  */
#if !defined __FORCE_NOGLIBC && (!defined _LIBC || defined __FORCE_GLIBC)
/* This macro indicates that the installed library is the GNU C Library.
   For historic reasons the value now is 6 and this will stay from now
   on.  The use of this variable is deprecated.  */
/* uClibc WARNING: leave these aligned to the left, don't put a space after '#', else
 * broken apps could fail the check. */
#undef  __GNU_LIBRARY__
#define __GNU_LIBRARY__ 6

/* Major and minor version number of the GNU C library package.  Use
   these macros to test for features in specific releases.  */
/* Don't do it, if you want to keep uClibc happy.  */
#define	__GLIBC__	2
#define	__GLIBC_MINOR__	2
#endif

#define __GLIBC_PREREQ(maj, min) \
	((__GLIBC__ << 16) + __GLIBC_MINOR__ >= ((maj) << 16) + (min))

#ifndef __UCLIBC__
/* Decide whether a compiler supports the long long datatypes.  */
#if defined __GNUC__ \
    || (defined __PGI && defined __i386__ ) \
    || (defined __INTEL_COMPILER && (defined __i386__ || defined __ia64__)) \
    || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L)
# define __GLIBC_HAVE_LONG_LONG	1
#endif
#endif

/* This is here only because every header file already includes this one.  */
#ifndef __ASSEMBLER__
# ifndef _SYS_CDEFS_H
#  include <sys/cdefs.h>
# endif

/* If we don't have __REDIRECT, prototypes will be missing if
   __USE_FILE_OFFSET64 but not __USE_LARGEFILE[64]. */
# if defined __USE_FILE_OFFSET64 && !defined __REDIRECT
#  define __USE_LARGEFILE	1
#  define __USE_LARGEFILE64	1
# endif

#endif	/* !ASSEMBLER */

/* Decide whether we can, and are willing to define extern inline
 * functions in headers, even if this results in a slightly bigger
 * code for user programs built against uclibc.
 * Enabled only in -O2 compiles, not -Os.
 * uclibc itself is usually built without __USE_EXTERN_INLINES,
 * remove "&& !defined __OPTIMIZE_SIZE__" part to do otherwise.
 */
#if __GNUC_PREREQ (2, 7) && defined __OPTIMIZE__ \
    && !defined __OPTIMIZE_SIZE__ && !defined __NO_INLINE__ \
    && (defined __extern_inline || defined __GNUC_GNU_INLINE__ || defined __GNUC_STDC_INLINE__)
# define __USE_EXTERN_INLINES	1
#endif

#ifdef _LIBC
#  undef _FILE_OFFSET_BITS
#  undef __USE_FILE_OFFSET64
# include <libc-internal.h>
#endif

#if defined(__UCLIBC_USE_TIME64__) || __TARGET_ARCH_BITS__ == 64
#define __USE_TIME_BITS64 1
#endif

#endif	/* features.h  */
