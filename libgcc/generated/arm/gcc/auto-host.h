/* auto-host.h.  Generated from config.in by configure.  */
/* Define if you want runtime assertions enabled. This is a cheap check. */
#define ENABLE_RUNTIME_CHECKING 1

/* Define 0/1 if your assembler supports CFI directives. */
#define HAVE_GAS_CFI_DIRECTIVE 1

/* Define 0/1 if your assembler supports .cfi_personality. */
#define HAVE_GAS_CFI_PERSONALITY_DIRECTIVE 1

/* Define 0/1 if your assembler supports .cfi_sections. */
#define HAVE_GAS_CFI_SECTIONS_DIRECTIVE 1

/* Define if your assembler and linker support .hidden. */
#define HAVE_GAS_HIDDEN 1

/* Define if your linker supports .eh_frame_hdr. */
#define HAVE_LD_EH_FRAME_HDR 1

/* Define if your target C library provides sys/sdt.h */
#undef HAVE_SYS_SDT_H

/* Define if your target C library provides the `dl_iterate_phdr' function. */
#define TARGET_DL_ITERATE_PHDR 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Define to `char *' if <sys/types.h> does not define. */


/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif


/* Define if you want to use __cxa_atexit, rather than atexit, to register C++
   destructors for local statics and global objects. This is essential for
   fully standards-compliant handling of destructors, but requires
   __cxa_atexit in libc. */
#ifndef USED_FOR_TARGET
#define DEFAULT_USE_CXA_ATEXIT 2
#endif

/* Define 0/1 if .init_array/.fini_array sections are available and working.
   */
#ifndef USED_FOR_TARGET
#if defined(__LIBGCC_INIT_ARRAY_SECTION_ASM_OP__)
#define HAVE_INITFINI_ARRAY_SUPPORT 1
#else
#define HAVE_INITFINI_ARRAY_SUPPORT 0
#endif
#endif
