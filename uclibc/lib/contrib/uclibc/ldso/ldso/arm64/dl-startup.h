/* vi: set sw=4 ts=4: */
/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2016 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

__asm__(
	"	.text\n"
	"	.globl  _start\n"
	"	.type   _start, %function\n"
	"	.hidden _start\n"
	"_start:\n"
	"	mov     x0, sp\n"
	"	bl      _dl_start\n"
	"	mov     x21, x0\n"
	"	ldr     x1, [sp]\n"
	"	add     x2, sp, #8\n"
	"	adrp    x4, _dl_skip_args\n"
	"	ldr     w4, [x4, #:lo12:_dl_skip_args]\n"
	"	cmp     w4, #0\n"
	"	beq     .L_stack_adjusted\n"
	"	sub     x1, x1, x4\n"
	"	str     x1, [sp]\n"
	"	mov     x3, x2\n"
	"	add     x4, x2, x4, lsl #3\n"
	"1:	ldr     x5, [x4], #8\n"
	"	str     x5, [x3], #8\n"
	"	cmp     x5, #0\n"
	"	bne     1b\n"
	"1:	ldr     x5, [x4], #8\n"
	"	str     x5, [x3], #8\n"
	"	cmp     x5, #0\n"
	"	bne     1b\n"
	"1:	ldp     x0, x5, [x4, #16]!\n"
	"	stp     x0, x5, [x3], #16\n"
	"	cmp     x0, #0\n"
	"	bne     1b\n"
	".L_stack_adjusted:\n"
	"	add     x3, x2, x1, lsl #3\n"
	"	add     x3, x3, #8\n"
	"	adrp    x0, _dl_fini\n"
	"	add     x0, x0, #:lo12:_dl_fini\n"
	"	br      x21\n"
	"\n\n"
	"	.size   _start, .-_start\n"
	".previous\n"
);


/* Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*)ARGS)+1)

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, ElfW(Sym) *symtab)
{
	(void) symtab;

	switch (ELF_R_TYPE(rpnt->r_info)) {
		case R_AARCH64_NONE:
			break;
		case R_AARCH64_RELATIVE:
			*reloc_addr = load_addr + rpnt->r_addend;
			break;
		case R_AARCH64_GLOB_DAT:
		case R_AARCH64_JUMP_SLOT:
		case R_AARCH64_ABS32:
		case R_AARCH64_ABS64:
			*reloc_addr = symbol_addr + rpnt->r_addend;
			 break;
		case R_AARCH64_COPY:
			/* break; */
		default:
			SEND_STDERR("Unsupported relocation type\n");
			_dl_exit(1);
	}
}
