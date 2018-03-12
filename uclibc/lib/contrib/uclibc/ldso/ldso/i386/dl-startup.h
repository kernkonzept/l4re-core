/* vi: set sw=4 ts=4: */
/*
 * Architecture specific code used by dl-startup.c
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 */
__asm__ (
    "	.text\n"
    "	.globl	_start\n"
    "	.type	_start,@function\n"
    "	.hidden	_start\n"
    "_start:\n"
/* start L4: fix for clang, or better fix according to C std */
    "	mov %esp, %eax\n"
/* end L4 */
    "	call _dl_start\n"
    "	# Save the user entry point address in %edi.\n"
    "	movl %eax, %edi\n"
    "	# Point %ebx at the GOT.\n"
    "	call 1f\n"
    "1:	popl	%ebx\n"
    "	addl $_GLOBAL_OFFSET_TABLE_+[.-1b], %ebx\n"
    "	# See if we were run as a command with the executable file\n"
    "	# name as an extra leading argument.\n"
    "	movl _dl_skip_args@GOTOFF(%ebx), %eax\n"
    "	# Pop the original argument count.\n"
    "	popl %edx\n"
    "	# Adjust the stack pointer to skip _dl_skip_args words.\n"
    "	leal (%esp,%eax,4), %esp\n"
    "	# Subtract _dl_skip_args from argc.\n"
    "	subl %eax, %edx\n"
    "	# Push argc back on the stack.\n"
    "	push %edx\n"
    "	# Pass our FINI ptr() to the user in %edx, as per ELF ABI.\n"
    "	leal _dl_fini@GOTOFF(%ebx), %edx\n"
    "	# Jump to the user's entry point.\n"
    "	jmp *%edi\n"
    "	.size	_start,.-_start\n"
    "	.previous\n"
);

/* start L4: fix for clang, or better fix according to C std */
#undef DL_START
#define DL_START(X) static void * __attribute_used__ __attribute__((regparm(1))) _dl_start(X)
/* end L4 */
/* Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.  */
/* start L4: fix for clang, or better fix according to C std */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) ARGS)+1)
/* L4: original code
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) & ARGS)+1)
*/
/* end L4 */

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, attribute_unused Elf32_Sym *symtab)
{
	switch (ELF_R_TYPE(rpnt->r_info))
	{
		case R_386_32:
			*reloc_addr += symbol_addr;
			break;
		case R_386_PC32:
			*reloc_addr += symbol_addr - (unsigned long) reloc_addr;
			break;
		case R_386_GLOB_DAT:
		case R_386_JMP_SLOT:
			*reloc_addr = symbol_addr;
			break;
		case R_386_RELATIVE:
			*reloc_addr += load_addr;
			break;
		default:
			_dl_exit(1);
	}
}
