/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB) \
switch(ELF_R_TYPE((RELP)->r_info)) { \
	case R_SPARC_32: \
	case R_SPARC_GLOB_DAT: \
		*REL = SYMBOL + (RELP)->r_addend; \
		break; \
	case R_SPARC_JMP_SLOT: \
		REL[1] = 0x03000000 | ((SYMBOL >> 10) & 0x3fffff); \
		REL[2] = 0x81c06000 | (SYMBOL & 0x3ff); \
		break; \
	case R_SPARC_NONE: \
	case R_SPARC_WDISP30: \
		break; \
	case R_SPARC_RELATIVE: \
		*REL += (unsigned int) LOAD + (RELP)->r_addend; \
		break; \
	default: \
		_dl_exit(1); \
}
