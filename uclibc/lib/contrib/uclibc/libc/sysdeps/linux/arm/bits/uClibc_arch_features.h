/*
 * Track misc arch-specific features that aren't config options
 */

#ifndef _BITS_UCLIBC_ARCH_FEATURES_H
#define _BITS_UCLIBC_ARCH_FEATURES_H

/* instruction used when calling abort() to kill yourself */
#define __UCLIBC_ABORT_INSTRUCTION__ "bl abort"

/* can your target use syscall6() for mmap ? */
#undef __UCLIBC_MMAP_HAS_6_ARGS__

/* does your target align 64bit values in register pairs ? (32bit arches only) */
#ifdef __ARM_EABI__
#define __UCLIBC_SYSCALL_ALIGN_64BIT__
#else
#undef __UCLIBC_SYSCALL_ALIGN_64BIT__
#endif

/* does your target have a broken create_module() ? */
#define __UCLIBC_BROKEN_CREATE_MODULE__

/* does your target have to worry about older [gs]etrlimit() ? */
#undef __UCLIBC_HANDLE_OLDER_RLIMIT__

/* does your target have an asm .set ? */
#define __UCLIBC_HAVE_ASM_SET_DIRECTIVE__

/* define if target doesn't like .global */
#undef __UCLIBC_ASM_GLOBAL_DIRECTIVE__

/* define if target supports .weak */
#define __UCLIBC_HAVE_ASM_WEAK_DIRECTIVE__

/* define if target supports .weakext */
#undef __UCLIBC_HAVE_ASM_WEAKEXT_DIRECTIVE__

/* needed probably only for ppc64 */
#undef __UCLIBC_HAVE_ASM_GLOBAL_DOT_NAME__

/* define if target supports CFI pseudo ops */
#undef __UCLIBC_HAVE_ASM_CFI_DIRECTIVES__

/* define if target supports IEEE signed zero floats */
#define __UCLIBC_HAVE_SIGNED_ZERO__

/* only weird assemblers generally need this */
#undef __UCLIBC_ASM_LINE_SEP__

#ifdef NOT_FOR_L4 // must disable this check below because it blows l4-lic-symbols.h
#ifdef __GNUC__
# define __need_uClibc_config_h
# include <bits/uClibc_config.h>
# undef __need_uClibc_config_h
# if defined __CONFIG_ARM_EABI__ && !defined __ARM_EABI__
#  error Your toolchain does not support EABI
# elif !defined __CONFIG_ARM_EABI__ && defined __ARM_EABI__
#  error Your toolchain was built for EABI, but you have chosen OABI
# endif
#endif
#endif

#endif /* _BITS_UCLIBC_ARCH_FEATURES_H */
