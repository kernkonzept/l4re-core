/* Any assembly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.
 * Copyright (C) 2005 by Joakim Tjernlund
 * Copyright (C) 2005 by Erik Andersen
 */


#include <sgidefs.h>
__asm__(""
    "	.text\n"
    "	.globl	_start\n"
    "	.ent	_start\n"
    "	.type	_start,@function\n"
    "	.hidden	_start\n"
    "_start:\n"
    "	.set noreorder\n"
    "	move	$25, $31\n"
    "	bal	0f\n"
    "	nop\n"
    "0:\n"
#if _MIPS_SIM == _MIPS_SIM_ABI32
    "	.cpload	$31\n"
#else	/* N32 || N64 */
    "	.cpsetup $31, $2, 0b\n"
#endif	/* N32 || N64 */
    "	move	$31, $25\n"
    "	.set reorder\n"
#if _MIPS_SIM == _MIPS_SIM_ABI64
    "	dla	$4, _DYNAMIC\n"
    "	sd	$4, -0x7ff0($28)\n"
#else	/* O32 || N32 */
    "	la	$4, _DYNAMIC\n"
    "	sw	$4, -0x7ff0($28)\n"
#endif	/* O32 || N32 */
    "	move	$4, $29\n"
#if _MIPS_SIM == _MIPS_SIM_ABI32
    "	subu	$29, 16\n"
#endif
#if _MIPS_SIM == _MIPS_SIM_ABI64
    "	dla	$8, .coff\n"
#else	/* O32 || N32 */
    "	la	$8, .coff\n"
#endif	/* O32 || N32 */
    "	bal	.coff\n"
    ".coff:\n"
#if _MIPS_SIM == _MIPS_SIM_ABI64
    "	dsubu	$8, $31, $8\n"
    "	dla	$25, _dl_start\n"
    "	daddu	$25, $8\n"
#else	/* O32 || N32 */
    "	subu	$8, $31, $8\n"
    "	la	$25, _dl_start\n"
    "	addu	$25, $8\n"
#endif	/* O32 || N32 */
    "	jalr	$25\n"
#if _MIPS_SIM == _MIPS_SIM_ABI32
    "	addiu	$29, 16\n"
#endif
    "	move	$16, $28\n"
    "	move	$17, $2\n"
#if _MIPS_SIM == _MIPS_SIM_ABI64
    "	ld	$2, _dl_skip_args\n"
    "	beq	$2, $0, 1f\n"
    "	ld	$4, 0($29)\n"
    "	dsubu	$4, $2\n"
    "	dsll	$2, 2\n"
    "	daddu	$29, $2\n"
    "	sd	$4, 0($29)\n"
    "1:\n"
    "	ld	$5, 0($29)\n"
    "	dla	$6, 8 ($29)\n"
    "	dsll	$7, $5, 2\n"
    "	daddu	$7, $7, $6\n"
    "	daddu	$7, $7, 4\n"
    "	and	$2, $29, -4 * 4\n"
    "	sd	$29, -8($2)\n"
    "	dsubu	$29, $2, 32\n"
    "	ld	$29, 24($29)\n"
    "	dla	$2, _dl_fini\n"
#else	/* O32 || N32 */
    "	lw	$2, _dl_skip_args\n"
    "	beq	$2, $0, 1f\n"
    "	lw	$4, 0($29)\n"
    "	subu	$4, $2\n"
    "	sll	$2, 2\n"
    "	addu	$29, $2\n"
    "	sw	$4, 0($29)\n"
    "1:\n"
    "	lw	$5, 0($29)\n"
    "	la	$6, 4 ($29)\n"
    "	sll	$7, $5, 2\n"
    "	addu	$7, $7, $6\n"
    "	addu	$7, $7, 4\n"
    "	and	$2, $29, -2 * 4\n"
    "	sw	$29, -4($2)\n"
    "	subu	$29, $2, 32\n"
#if _MIPS_SIM == _MIPS_SIM_ABI32
    "	.cprestore 16\n"
#endif
    "	lw	$29, 28($29)\n"
    "	la	$2, _dl_fini\n"
#endif	/* O32 || N32 */
    "	move	$25, $17\n"
    "	jr	$25\n"
    ".end	_start\n"
    ".size	_start, . -_start\n"
    "\n\n"
    "\n\n"
    ".previous\n"
);

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long *) ARGS)+1)


/* We can't call functions earlier in the dl startup process */
#define NO_FUNCS_BEFORE_BOOTSTRAP

#include "dl-bootstrap.h"
