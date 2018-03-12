/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include "elf.h"

/*
 * Define this if the system uses RELOCA.
 */
#define ELF_USES_RELOCA

/*
 * Dynamic Linking ABI for ARCompact ISA
 *
 *                      PLT
 *        --------------------------------
 *        |  ld r11, [pcl, off-to-GOT[1] |  0   (20 bytes)
 *        |                              |  4
 * plt0   |  ld r10, [pcl, off-to-GOT[2] |  8
 *        |                              | 12
 *        |  j [r10]                     | 16
 *        --------------------------------
 *        |    Base address of GOT       | 20
 *        --------------------------------
 *        |  ld r12, [pcl, off-to-GOT[3] | 24   (12 bytes each)
 * plt1   |                              |
 *        |  j_s.d  [r12]                | 32
 *        |  mov_s  r12, pcl             | 34
 *        --------------------------------
 *        |                              | 36
 *        ~                              ~
 *        ~                              ~
 *        |                              |
 *        --------------------------------
 *
 *             GOT
 *        --------------
 *        |    [0]     |
 *        --------------
 *        |    [1]     |  Module info - setup by ldso
 *        --------------
 *        |    [2]     |  resolver entry point
 *        --------------
 *        |    [3]     |
 *        |    ...     |  Runtime address for function symbols
 *        |    [f]     |
 *        --------------
 *        |    [f+1]   |
 *        |    ...     |  Runtime address for data symbols
 *        |    [last]  |
 *        --------------
 */

/*
 * Initialization sequence for a GOT.
 * Caller elf_resolve() seeds @GOT_BASE from DT_PLTGOT - which essentially is
 * pointer to first PLT entry. The actual GOT base is 5th word in PLT
 *
 */
#define INIT_GOT(GOT_BASE,MODULE)					\
do {									\
	unsigned long *__plt_base = (unsigned long *)GOT_BASE;		\
	GOT_BASE = (unsigned long *)(__plt_base[5] +			\
		                     (unsigned long)MODULE->loadaddr);	\
	GOT_BASE[1] = (unsigned long) MODULE;				\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve;		\
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_ARCOMPACT
#undef  MAGIC2

/* Used for error messages */
#define ELF_TARGET "ARC"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt,
					 unsigned int plt_pc);

extern unsigned __udivmodsi4(unsigned, unsigned) attribute_hidden;

#define do_rem(result, n, base)  ((result) =				\
									\
	__builtin_constant_p (base) ? (n) % (unsigned) (base) :		\
	__extension__ ({						\
		register unsigned r1 __asm__ ("r1") = (base);		\
									\
		__asm__("bl.d @__udivmodsi4` mov r0,%1"			\
		: "=r" (r1)						\
	        : "r" (n), "r" (r1)					\
	        : "r0", "r2", "r3", "r4", "lp_count", "blink", "cc");	\
									\
		r1;							\
	})								\
)

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_ARC_JMP_SLOT) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_ARC_COPY) * ELF_RTYPE_CLASS_COPY))

/*
 * Get the runtime address of GOT[0]
 */
static __always_inline Elf32_Addr elf_machine_dynamic(void)
{
	Elf32_Addr dyn;

	__asm__("ld %0,[pcl,_DYNAMIC@gotpc]\n\t" : "=r" (dyn));
	return dyn;

/*
 * Another way would have been to simply return GP, which due to some
 * PIC reference would be automatically setup by gcc in caller
 *	register Elf32_Addr *got __asm__ ("gp"); return *got;
 */
}

/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr elf_machine_load_address(void)
{
    /* To find the loadaddr we subtract the runtime addr of any symbol
     * say _dl_start from it's build-time addr.
     */
	Elf32_Addr addr, tmp;
	__asm__ (
        "ld  %1, [pcl, _dl_start@gotpc] ;build addr of _dl_start   \n"
        "add %0, pcl, _dl_start-.+(.&2) ;runtime addr of _dl_start \n"
        "sub %0, %0, %1                 ;delta                     \n"
        : "=&r" (addr), "=r"(tmp)
    );
	return addr;
}

static __always_inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	 Elf32_Rel * rpnt = (void *) rel_addr;
	--rpnt;
	do {
		Elf32_Addr *const reloc_addr = (void *) (load_off + (++rpnt)->r_offset);
		*reloc_addr += load_off;
	} while (--relative_count);
}
