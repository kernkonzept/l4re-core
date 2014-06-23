#ifndef _LINUX_ARC_SYSDEP_H
#define _LINUX_ARC_SYSDEP_H 1

#include <features.h>
#include <libc-internal.h>

#ifdef	__ASSEMBLER__

#define ENTRY(nm)		\
	.text `			\
	.align 4 `		\
	.globl nm `		\
	.type nm,@function `	\
nm:

#define END(name)	.size name,.-name

#endif /* __ASSEMBLER __*/

#include <common/sysdep.h>

/* Pointer mangling is not yet supported  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#endif
