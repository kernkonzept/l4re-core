__asm__ ("\
	.text\n\
	.globl _start\n\
	.type _start, %function\n\
_start:\n\
        mov r4, sp\n\
        br _dl_start\n\
	mov r16, r4\n\
        jmp r16\n\
");

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) ARGS)+1)

/* The ld.so library requires relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, attribute_unused Elf32_Sym *symtab)
{
	switch (ELF_R_TYPE(rpnt->r_info)) {
		case R_NIOS2_RELATIVE:
			*reloc_addr = load_addr + rpnt->r_addend;
			break;
		default:
			_dl_exit(1);
			break;
	}
}
