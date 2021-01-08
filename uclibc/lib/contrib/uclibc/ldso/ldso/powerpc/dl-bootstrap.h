/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB) \
	{int type=ELF_R_TYPE((RELP)->r_info);		\
	 Elf32_Addr finaladdr=(SYMBOL)+(RELP)->r_addend;\
	if (type==R_PPC_RELATIVE) {			\
		*REL=(Elf32_Word)(LOAD)+(RELP)->r_addend;\
	} else if (type==R_PPC_ADDR32 || type==R_PPC_GLOB_DAT) {\
		*REL=finaladdr;				\
	} else if (type==R_PPC_JMP_SLOT) {		\
		Elf32_Sword delta=finaladdr-(Elf32_Word)(REL);\
		*REL=OPCODE_B(delta);			\
		PPC_DCBST(REL); PPC_SYNC; PPC_ICBI(REL);\
	} else {					\
		_dl_exit(100+ELF_R_TYPE((RELP)->r_info));\
	}						\
	}

