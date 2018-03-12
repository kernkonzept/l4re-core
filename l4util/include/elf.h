/**
 * \file
 * \brief	ELF definition
 *
 * \date	08/18/2000
 * \author	Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *		Alexander Warg <aw11@os.inf.tu-dresden.de> 
 *
 * Many structs from
 *   "Executable and Linkable Format (ELF)",
 *    Portable Formats Specification, Version 1.1
 * and
 *   "System V Application Binary Interface - DRAFT - April 29, 1998"
 *   The Santa Cruz Operation, Inc.
 *   (see http://www.sco.com/developer/gabi/contents.html)
 *
 *   \ingroup l4util_elf
 */
/*
 * (c) 2008-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

/* (c) 2003-2006 Technische Universitaet Dresden
 * This file is part of the exec package, which is distributed under
 * the terms of the GNU General Public License 2. Please see the
 * COPYING file for details. */

#ifndef _L4_EXEC_ELF_H
#define _L4_EXEC_ELF_H

#include <l4/sys/l4int.h>

/**
 * \defgroup l4util_elf ELF binary format
 * \ingroup l4util_api
 * \brief Functions and types related to ELF binaries.
 */

/** \name ELF types
 * \ingroup l4util_elf
 */
/*@{*/
typedef l4_uint32_t	Elf32_Addr;   /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_uint32_t	Elf32_Off;    /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_uint16_t	Elf32_Half;   /**< size 2 align 2 \ingroup l4util_elf*/
typedef l4_uint32_t	Elf32_Word;   /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_int32_t	Elf32_Sword;  /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_uint64_t	Elf64_Addr;   /**< size 8 align 8 \ingroup l4util_elf*/
typedef	l4_uint64_t	Elf64_Off;    /**< size 8 align 8 \ingroup l4util_elf*/
typedef l4_uint16_t	Elf64_Half;   /**< size 2 align 2 \ingroup l4util_elf*/
typedef	l4_uint32_t	Elf64_Word;   /**< size 4 align 4 \ingroup l4util_elf*/
typedef	l4_int32_t	Elf64_Sword;  /**< size 4 align 4 \ingroup l4util_elf*/
typedef	l4_uint64_t	Elf64_Xword;  /**< size 8 align 8 \ingroup l4util_elf*/
typedef	l4_int64_t	Elf64_Sxword; /**< size 8 align 8 \ingroup l4util_elf*/
/*@}*/

#if L4_MWORD_BITS == 64
/**
 * \brief Use 64 or 32 bits types depending on the target architecture.
 * \ingroup l4util_elf
 */
#define ElfW(type)      _ElfW(Elf, 64, type)
#else
#define ElfW(type)      _ElfW(Elf, 32, type)
#endif
#define _ElfW(e,w,t)    __ElfW(e, w, _##t)
#define __ElfW(e,w,t)   e##w##t

#ifdef ARCH_x86
#define L4_ARCH_EI_DATA      ELFDATA2LSB
#define L4_ARCH_E_MACHINE    EM_386
#define L4_ARCH_EI_CLASS     ELFCLASS32
#else
#ifdef ARCH_amd64
#define L4_ARCH_EI_DATA      ELFDATA2LSB
#define L4_ARCH_E_MACHINE    EM_X86_64
#define L4_ARCH_EI_CLASS     ELFCLASS64
#else
#ifdef ARCH_arm
#define L4_ARCH_EI_DATA      ELFDATA2LSB
#define L4_ARCH_E_MACHINE    EM_ARM
#define L4_ARCH_EI_CLASS     ELFCLASS32
#else
#ifdef ARCH_ppc32
#define L4_ARCH_EI_DATA      ELFDATA2MSB
#define L4_ARCH_E_MACHINE    EM_PPC
#define L4_ARCH_EI_CLASS     ELFCLASS32
#else
#ifdef ARCH_sparc
#define L4_ARCH_EI_DATA      ELFDATA2MSB
#define L4_ARCH_E_MACHINE    EM_SPARC
#define L4_ARCH_EI_CLASS     ELFCLASS32
#elif defined(ARCH_mips)
#define L4_ARCH_EI_DATA      ELFDATA2LSB
#define L4_ARCH_E_MACHINE    EM_MIPS
#ifdef __mips64
#define L4_ARCH_EI_CLASS     ELFCLASS64
#else
#define L4_ARCH_EI_CLASS     ELFCLASS32
#endif
#else
#warning elf.h: Unsupported build architecture!
#endif
#endif
#endif
#endif
#endif


/*************************************/
/* ELF Header - figure 1-3, page 1-3 */
/*************************************/

/** \addtogroup l4util_elf */

/*@{*/

#define EI_NIDENT 16			/**< \brief number of characters */
/**
 * \brief ELF32 header.
 */
typedef struct {
    unsigned char	e_ident[EI_NIDENT];
    Elf32_Half		e_type;		/**< type of ELF file */
    Elf32_Half		e_machine;	/**< required architecture */
    Elf32_Word		e_version;	/**< file version */
    Elf32_Addr		e_entry;	/**< initial eip */
    Elf32_Off		e_phoff;	/**< offset of program header table */
    Elf32_Off		e_shoff;	/**< offset of file header table */
    Elf32_Word		e_flags;	/**< processor-specific flags */
    Elf32_Half		e_ehsize;	/**< size of ELF header */
    Elf32_Half		e_phentsize;	/**< size of program header entry */
    Elf32_Half		e_phnum;	/**< # of entries in prog. head. tab. */
    Elf32_Half		e_shentsize;	/**< size of section header entry */
    Elf32_Half		e_shnum;	/**< # of entries in sect. head. tab. */
    Elf32_Half		e_shstrndx;	/**< sect.head.tab.idx of strtab */
} Elf32_Ehdr;

/**
 * \brief ELF64 header.
 */
typedef struct {
    unsigned char	e_ident[EI_NIDENT];
    Elf64_Half		e_type;		/**< \brief type of ELF file */
    Elf64_Half		e_machine;	/**< \brief required architecture */
    Elf64_Word		e_version;	/**< \brief file version */
    Elf64_Addr		e_entry;	/**< \brief initial eip */
    Elf64_Off		e_phoff;	/**< \brief offset of program header table */
    Elf64_Off		e_shoff;	/**< \brief offset of file header table */
    Elf64_Word		e_flags;	/**< \brief processor-specific flags */
    Elf64_Half		e_ehsize;	/**< \brief size of ELF header */
    Elf64_Half		e_phentsize;	/**< \brief size of program header entry */
    Elf64_Half		e_phnum;	/**< \brief # of entries in prog. head. tab. */
    Elf64_Half		e_shentsize;	/**< \brief size of section header entry */
    Elf64_Half		e_shnum;	/**< \brief # of entries in sect. head. tab. */
    Elf64_Half		e_shstrndx;	/**< \brief sect.head.tab.idx of strtab */
} Elf64_Ehdr;

#define EI_CLASS	4 /**< \brief ELF class byte index */
#define ELFCLASSNONE	0 /**< \brief Invalid ELF class */
#define ELFCLASS32	1 /**< \brief 32-bit objects */
#define ELFCLASS64	2 /**< \brief 64-bit objects */
#define ELFCLASSNUM	3 /**< \brief Mask for 32-bit or 64-bit class */

#define EI_DATA		5		/**< Data encoding byte index */
#define ELFDATANONE	0		/**< Invalid data encoding */
#define ELFDATA2LSB	1		/**< 2's complement, little endian */
#define ELFDATA2MSB	2		/**< 2's complement, big endian */
#define ELFDATANUM	3

#define EI_VERSION	6		/**< File version byte index */
					/**< Value must be EV_CURRENT */

#define EI_OSABI	7		/**< OS ABI identification */
#define ELFOSABI_NONE		0	/**< UNIX System V ABI */
#define ELFOSABI_SYSV		0	/**< Alias.  */
#define ELFOSABI_HPUX		1	/**< HP-UX */
#define ELFOSABI_NETBSD		2	/**< NetBSD.  */
#define ELFOSABI_LINUX		3	/**< Linux.  */
#define ELFOSABI_SOLARIS	6	/**< Sun Solaris.  */
#define ELFOSABI_AIX		7	/**< IBM AIX.  */
#define ELFOSABI_IRIX		8	/**< SGI Irix.  */
#define ELFOSABI_FREEBSD	9	/**< FreeBSD.  */
#define ELFOSABI_TRU64		10	/**< Compaq TRU64 UNIX.  */
#define ELFOSABI_MODESTO	11	/**< Novell Modesto.  */
#define ELFOSABI_OPENBSD	12	/**< OpenBSD.  */
#define ELFOSABI_ARM		97	/**< ARM */
#define ELFOSABI_STANDALONE	255	/**< Standalone (embedded) application */

#define EI_ABIVERSION	8		/**< ABI version */

#define EI_PAD		9		/**< Byte index of padding bytes */

/* object file type - page 1-3 (e_type) */

#define ET_NONE		0	/**< no file type */
#define ET_REL		1	/**< relocatable file */
#define ET_EXEC		2	/**< executable file */
#define ET_DYN		3	/**< shared object file */
#define ET_CORE		4	/**< core file */
#define ET_LOPROC	0xff00	/**< processor-specific */
#define ET_HIPROC	0xffff	/**< processor-specific */

/* required architecture - page 1-4 (e_machine) */

#define EM_NONE		0	/**< no machine */
#define EM_M32		1	/**< AT&T WE 32100 */
#define EM_SPARC	2	/**< SPARC */
#define EM_386		3	/**< Intel 80386 */
#define EM_68K		4	/**< Motorola 68000 */
#define EM_88K		5	/**< Motorola 88000 */
#define EM_860		7	/**< Intel 80860 */
#define EM_MIPS		8	/**< MIPS RS3000 big-endian */
#define EM_MIPS_RS4_BE	10	/**< MIPS RS4000 big-endian */
#define EM_SPARC64	11	/**< SPARC 64-bit */
#define EM_PARISC	15	/**< HP PA-RISC */
#define EM_VPP500	17	/**< Fujitsu VPP500 */
#define EM_SPARC32PLUS	18	/**< Sun's V8plus */
#define EM_960		19	/**< Intel 80960 */
#define EM_PPC		20	/**< PowerPC */
#define EM_V800		36	/**< NEC V800 */
#define EM_FR20		37	/**< Fujitsu FR20 */
#define EM_RH32		38	/**< TRW RH-32 */
#define EM_RCE		39	/**< Motorola RCE */
#define EM_ARM		40	/**< Advanced RISC Machines ARM */
#define EM_ALPHA	41	/**< Digital Alpha */
#define EM_SH		42	/**< Hitachi SuperH */
#define EM_SPARCV9	43	/**< SPARC v9 64-bit */
#define EM_TRICORE	44	/**< Siemens Tricore embedded processor */
#define EM_ARC		45	/**< Argonaut RISC Core, Argonaut Techn Inc. */
#define EM_H8_300	46	/**< Hitachi H8/300 */
#define EM_H8_300H	47	/**< Hitachi H8/300H */
#define EM_H8S		48	/**< Hitachi H8/S */
#define EM_H8_500	49	/**< Hitachi H8/500 */
#define EM_IA_64	50	/**< HP/Intel IA-64 */
#define EM_MIPS_X	51	/**< Stanford MIPS-X */
#define EM_COLDFIRE	52	/**< Motorola Coldfire */
#define EM_68HC12	53	/**< Motorola M68HC12 */
#define EM_X86_64	62      /**< Advanced Micro Devices x86-64 */

#if 0
#define EM_ALPHA	0x9026	/* interium value used by Linux until the
				   committee comes up with a final number */
#define EM_S390		0xA390	/* interium value used for IBM S390 */
#endif

/* object file version - page 1-4 (e_version) */

#define EV_NONE		0	/**< Invalid version */
#define EV_CURRENT	1	/**< Current version */

/* e_ident[] Identification Indexes - figure 1-4, page 1-5 */

#define EI_MAG0		0	/**< file id */
#define EI_MAG1		1	/**< file id */
#define EI_MAG2		2	/**< file id */
#define EI_MAG3		3	/**< file id */
#define EI_CLASS	4	/**< file class */
#define EI_DATA		5	/**< data encoding */
#define EI_VERSION	6	/**< file version */
#define EI_OSABI	7	/**< Operating system / ABI identification */
#define EI_ABIVERSION	8	/**< ABI version */
#define EI_PAD		9	/**< start of padding bytes */

/* magic number - page 1-5 */

#define ELFMAG0		0x7f	/**< e_ident[EI_MAG0] */
#define ELFMAG1		'E'	/**< e_ident[EI_MAG1] */
#define ELFMAG2		'L'	/**< e_ident[EI_MAG2] */
#define ELFMAG3		'F'	/**< e_ident[EI_MAG3] */

/* file class or capacity - page 1-6 */

#define ELFCLASSNONE	0	/**< Invalid class */
#define ELFCLASSS32	1	/**< 32-bit object */
#define ELFCLASSS64	2	/**< 64-bit object */

/* data encoding - page 1-6 */

#define ELFDATANONE	0	/**< invalid data encoding */
#define ELFDATA2LSB	1	/**< 0x01020304 => [ 0x04|0x03|0x02|0x01 ] */
#define ELFDATA2MSB	2	/**< 0x01020304 => [ 0x01|0x02|0x03|0x04 ] */

/* Identify operating system and ABI to which the object is targeted */

#define ELFOSABI_SYSV	0	/**< UNIX System V ABI (this specification) */
#define ELFOSABI_HPUX	1	/**< HP-UX operating system */
#define ELFOSABI_STANDALONE 255  /**< Standalone (embedded) application */


/***********************/
/* Sections - page 1-8 */
/***********************/

/* special section indexes */

#define SHN_UNDEF	0		/**< undefined section header entry */
#define SHN_LORESERVE	0xff00		/**< lower bound of reserved indexes */
#define SHN_LOPROC	0xff00		/**< lower bound of proc spec entr */
#define SHN_HIPROC	0xff1f		/**< upper bound of proc spec entr */
#define SHN_ABS		0xfff1		/**< absolute values for ref */
#define SHN_COMMON	0xfff2		/**< common symbols */
#define SHN_HIRESERVE	0xffff		/**< upper bound of reserved indexes */

/** ELF32 section header - figure 1-9, page 1-9 */
typedef struct {
    Elf32_Word		sh_name;	/**< name of sect (idx into strtab) */
    Elf32_Word		sh_type;	/**< section's type */
    Elf32_Word		sh_flags;	/**< section's flags */
    Elf32_Addr		sh_addr;	/**< memory address of section */
    Elf32_Off		sh_offset;	/**< file offset of section */
    Elf32_Word		sh_size;	/**< file size of section */
    Elf32_Word		sh_link;	/**< idx to associated header section */
    Elf32_Word		sh_info;	/**< extra info of header section */
    Elf32_Word		sh_addralign;	/**< address alignment constraints */
    Elf32_Word		sh_entsize;	/**< size of entry if sect is table */
} Elf32_Shdr;

/** ELF64 section header */
typedef struct {
    Elf64_Word		sh_name;	/**< name of sect (idx into strtab) */
    Elf64_Word		sh_type;	/**< section's type */
    Elf64_Xword		sh_flags;	/**< section's flags */
    Elf64_Addr		sh_addr;	/**< memory address of section */
    Elf64_Off		sh_offset;	/**< file offset of section */
    Elf64_Xword		sh_size;	/**< file size of section */
    Elf64_Word		sh_link;	/**< idx to associated header section */
    Elf64_Word		sh_info;	/**< extra info of header section */
    Elf64_Xword		sh_addralign;	/**< address alignment constraints */
    Elf64_Xword		sh_entsize;	/**< size of entry if sect is table */
} Elf64_Shdr;

/* section type - figure 1-10, page 1-10 */

#define SHT_NULL	0
#define SHT_PROGBITS	1
#define SHT_SYMTAB	2
#define SHT_STRTAB	3
#define SHT_RELA	4
#define SHT_HASH	5
#define SHT_DYNAMIC	6
#define SHT_NOTE	7
#define SHT_NOBITS	8
#define SHT_REL		9
#define SHT_SHLIB	10
#define SHT_DYNSYM	11
#define SHT_INIT_ARRAY	  14		/**< Array of constructors */
#define SHT_FINI_ARRAY	  15		/**< Array of destructors */
#define SHT_PREINIT_ARRAY 16		/**< Array of pre-constructors */
#define SHT_GROUP	  17		/**< Section group */
#define SHT_SYMTAB_SHNDX  18		/**< Extended section indeces */
#define	SHT_NUM		  19		/**< Number of defined types.  */
#define SHT_LOOS	0x60000000
#define SHT_HIOS	0x6fffffff
#define SHT_LOPROC	0x70000000
#define SHT_HIPROC	0x7fffffff
#define SHT_LOUSER	0x80000000
#define SHT_HIUSER	0xffffffff

/* section attribute flags - page 1-12, figure 1-12 */

#define SHF_WRITE	0x1		/**< writeable during execution */
#define SHF_ALLOC	0x2		/**< section occupies virt memory */
#define SHF_EXECINSTR	0x4		/**< code section */
#define SHF_MERGE	0x10	        /**< Might be merged */
#define SHF_STRINGS	0x20	        /**< Contains nul-terminated strings */
#define SHF_INFO_LINK	0x40	        /**< `sh_info' contains SHT index */
#define SHF_LINK_ORDER	0x80	        /**< Preserve order after combining */
#define SHF_OS_NONCONFORMING 0x100	/**< Non-standard OS specific handling
					     required */
#define SHF_GROUP	0x200 	        /**< Section is member of a group.  */
#define SHF_TLS		0x400	        /**< Section hold thread-local data.  */
#define SHF_MASKOS	0x0ff00000	/**< OS-specific.  */
#define SHF_MASKPROC	0xf0000000	/**< proc spec mask */


/*****************************************/
/* Program Header - figure 2-1, page 2-2 */
/*****************************************/

/** ELF32 program header */
typedef struct {
    Elf32_Word		p_type;		/**< type of program section */
    Elf32_Off		p_offset;	/**< file offset of program section */
    Elf32_Addr		p_vaddr;	/**< memory address of prog section */
    Elf32_Addr		p_paddr;	/**< physical address (ignored) */
    Elf32_Word		p_filesz;	/**< file size of program section */
    Elf32_Word		p_memsz;	/**< memory size of program section */
    Elf32_Word		p_flags;	/**< flags */
    Elf32_Word		p_align;	/**< alignment of section */
} Elf32_Phdr;

/** ELF64 program header */
typedef struct {
    Elf64_Word		p_type;		/**< type of program section */
    Elf64_Word		p_flags;	/**< flags */
    Elf64_Off		p_offset;	/**< file offset of program section */
    Elf64_Addr		p_vaddr;	/**< memory address of prog section */
    Elf64_Addr		p_paddr;	/**< physical address (ignored) */
    Elf64_Xword		p_filesz;	/**< file size of program section */
    Elf64_Xword		p_memsz;	/**< memory size of program section */
    Elf64_Xword		p_align;	/**< alignment of section */
} Elf64_Phdr;

/* segment types - figure 2-2, page 2-3 */

#define PT_NULL		0	   /**< array is unused */
#define PT_LOAD		1	   /**< loadable */
#define PT_DYNAMIC	2	   /**< dynamic linking information */
#define PT_INTERP	3	   /**< path to interpreter */
#define PT_NOTE		4	   /**< auxiliary information */
#define PT_SHLIB	5	   /**< reserved */
#define PT_PHDR		6	   /**< location of the pht itself */
#define PT_TLS		7	   /**< Thread-local storage segment */
#define	PT_NUM		8	   /**< Number of defined types */
#define PT_LOOS		0x60000000 /**< os spec. */
#define PT_HIOS		0x6fffffff /**< os spec. */
#define PT_LOPROC	0x70000000 /**< processor spec. */
#define PT_HIPROC	0x7fffffff /**< processor spec. */

#define PT_GNU_EH_FRAME (PT_LOOS + 0x474e550) /**< EH frame information. */
#define PT_GNU_STACK    (PT_LOOS + 0x474e551) /**< Flags for stack. */
#define PT_GNU_RELRO    (PT_LOOS + 0x474e552) /**< Read only after reloc. */

#define PT_L4_STACK     (PT_LOOS + 0x12) /**< Address of the stack. */
#define PT_L4_KIP       (PT_LOOS + 0x13) /**< Address of the KIP. */
#define PT_L4_AUX       (PT_LOOS + 0x14) /**< Address of the AUX strcutures. */

/* segment permissions - page 2-3 */

#define PF_X		0x1
#define PF_W		0x2
#define PF_R		0x4
#define PF_MASKOS	0x0ff00000
#define PF_MASKPROC	0x7fffffff

/* Legal values for note segment descriptor types for core files. */

#define NT_PRSTATUS	1	/**< Contains copy of prstatus struct */
#define NT_FPREGSET	2	/**< Contains copy of fpregset struct */
#define NT_PRPSINFO	3	/**< Contains copy of prpsinfo struct */
#define NT_PRXREG	4	/**< Contains copy of prxregset struct */
#define NT_TASKSTRUCT	4	/**< Contains copy of task structure */
#define NT_PLATFORM	5	/**< String from sysinfo(SI_PLATFORM) */
#define NT_AUXV		6	/**< Contains copy of auxv array */
#define NT_GWINDOWS	7	/**< Contains copy of gwindows struct */
#define NT_ASRS		8	/**< Contains copy of asrset struct */
#define NT_PSTATUS	10	/**< Contains copy of pstatus struct */
#define NT_PSINFO	13	/**< Contains copy of psinfo struct */
#define NT_PRCRED	14	/**< Contains copy of prcred struct */
#define NT_UTSNAME	15	/**< Contains copy of utsname struct */
#define NT_LWPSTATUS	16	/**< Contains copy of lwpstatus struct */
#define NT_LWPSINFO	17	/**< Contains copy of lwpinfo struct */
#define NT_PRFPXREG	20	/**< Contains copy of fprxregset struct*/

/* Legal values for the note segment descriptor types for object files.  */

#define NT_VERSION	1	/**< Contains a version string.  */

/* Dynamic structure - figure 2-9, page 2-12 */

/** ELF32 dynamic entry */
typedef struct {
    Elf32_Sword		d_tag;	/**< see DT_ values */
    union {
	Elf32_Word	d_val;	/**< integer values with various interpret. */
	Elf32_Addr	d_ptr;	/**< program virtual addresses */
    } d_un;
} Elf32_Dyn;

/** ELF64 dynamic entry */
typedef struct {
    Elf64_Sxword	d_tag;	/**< see DT_ values */
    union {
	Elf64_Xword	d_val;	/**< integer values with various interpret. */
	Elf64_Addr	d_ptr;	/**< program virtual addresses */
    } d_un;
} Elf64_Dyn;

/** Dynamic Array Tags, d_tag - figure 2-10, page 2-12 */

#define DT_NULL		0	/**< end of _DYNAMIC array */
#define DT_NEEDED	1	/**< name of a needed library */
#define DT_PLTRELSZ	2	/**< total size of relocation entry */
#define DT_PLTGOT	3	/**< address assoc with prog link table */
#define DT_HASH		4	/**< address of symbol hash table */
#define DT_STRTAB	5	/**< address of string table */
#define DT_SYMTAB	6	/**< address of symbol table */
#define DT_RELA		7	/**< address of relocation table */
#define DT_RELASZ	8	/**< total size of relocation table */
#define DT_RELAENT	9	/**< size of DT_RELA relocation entry */
#define DT_STRSZ	10	/**< size of the string table */
#define DT_SYMENT	11	/**< size of a symbol table entry */
#define DT_INIT		12	/**< address of initialization function */
#define DT_FINI		13	/**< address of termination function */
#define DT_SONAME	14	/**< name of the shared object */
#define DT_RPATH	15	/**< search library path */
#define DT_SYMBOLIC	16	/**< alter symbol resolution algorithm */
#define DT_REL		17	/**< address of relocation table */
#define DT_RELSZ	18	/**< total size of DT_REL relocation table */
#define DT_RELENT	19	/**< size of the DT_REL relocation entry */
#define DT_PTRREL	20	/**< type of relocation entry */
#define DT_DEBUG	21	/**< for debugging purposes */
#define DT_TEXTREL	22	/**< at least on entry changes r/o section */
#define DT_JMPREL	23	/**< address of relocation entries */
#define	DT_BIND_NOW	24	/**< Process relocations of object */
#define	DT_INIT_ARRAY	25	/**< Array with addresses of init fct */
#define	DT_FINI_ARRAY	26	/**< Array with addresses of fini fct */
#define	DT_INIT_ARRAYSZ	27	/**< Size in bytes of DT_INIT_ARRAY */
#define	DT_FINI_ARRAYSZ	28	/**< Size in bytes of DT_FINI_ARRAY */
#define DT_RUNPATH	29	/**< Library search path */
#define DT_FLAGS	30	/**< Flags for the object being loaded */
#define DT_ENCODING	32	/**< Start of encoded range */
#define DT_PREINIT_ARRAY 32	/**< Array with addresses of preinit fct*/
#define DT_PREINIT_ARRAYSZ 33	/**< size in bytes of DT_PREINIT_ARRAY */
#define	DT_NUM		34	/**< Number used */
#define DT_LOOS		0x6000000d /**< Start of OS-specific */
#define DT_HIOS		0x6ffff000 /**< End of OS-specific */
#define DT_LOPROC	0x70000000 /**< processor spec. */
#define DT_HIPROC	0x7fffffff /**< processor spec. */

/* Values of `d_un.d_val' in the DT_FLAGS entry.  */
#define DF_ORIGIN	0x00000001	/**< Object may use DF_ORIGIN */
#define DF_SYMBOLIC	0x00000002	/**< Symbol resolutions starts here */
#define DF_TEXTREL	0x00000004	/**< Object contains text relocations */
#define DF_BIND_NOW	0x00000008	/**< No lazy binding for this object */
#define DF_STATIC_TLS	0x00000010	/**< Module uses the static TLS model */

/* State flags selectable in the `d_un.d_val' element of the DT_FLAGS_1
   entry in the dynamic section.  */
#define DF_1_NOW	0x00000001	/**< Set RTLD_NOW for this object.  */
#define DF_1_GLOBAL	0x00000002	/**< Set RTLD_GLOBAL for this object.  */
#define DF_1_GROUP	0x00000004	/**< Set RTLD_GROUP for this object.  */
#define DF_1_NODELETE	0x00000008	/**< Set RTLD_NODELETE for this object.*/
#define DF_1_LOADFLTR	0x00000010	/**< Trigger filtee loading at runtime.*/
#define DF_1_INITFIRST	0x00000020	/**< Set RTLD_INITFIRST for this object*/
#define DF_1_NOOPEN	0x00000040	/**< Set RTLD_NOOPEN for this object.  */
#define DF_1_ORIGIN	0x00000080	/**< $ORIGIN must be handled.  */
#define DF_1_DIRECT	0x00000100	/**< Direct binding enabled.  */
#define DF_1_TRANS	0x00000200
#define DF_1_INTERPOSE	0x00000400	/**< Object is used to interpose.  */
#define DF_1_NODEFLIB	0x00000800	/**< Ignore default lib search path.  */
#define DF_1_NODUMP	0x00001000	/**< Object can't be dldump'ed.  */
#define DF_1_CONFALT	0x00002000	/**< Configuration alternative created.*/
#define DF_1_ENDFILTEE	0x00004000	/**< Filtee terminates filters search. */
#define	DF_1_DISPRELDNE	0x00008000	/**< Disp reloc applied at build time. */
#define	DF_1_DISPRELPND	0x00010000	/**< Disp reloc applied at run-time.  */

/* Flags for the feature selection in DT_FEATURE_1.  */
#define DTF_1_PARINIT	0x00000001
#define DTF_1_CONFEXP	0x00000002

/* Flags in the DT_POSFLAG_1 entry effecting only the next DT_* entry.  */
#define DF_P1_LAZYLOAD	0x00000001	/**< Lazyload following object.  */
#define DF_P1_GROUPPERM	0x00000002	/**< Symbols from next object are not
					   generally available.  */

/* Relocation - page 1-21, figure 1-20 */

typedef struct {
    Elf32_Addr		r_offset;
    Elf32_Word		r_info;
} Elf32_Rel;

typedef struct {
    Elf32_Addr		r_offset;
    Elf32_Word		r_info;
    Elf32_Sword		r_addend;
} Elf32_Rela;

typedef struct {
    Elf64_Addr		r_offset;
    Elf64_Xword		r_info;
} Elf64_Rel;

typedef struct {
    Elf64_Addr		r_offset;
    Elf64_Xword		r_info;
    Elf64_Sxword	r_addend;
} Elf64_Rela;

#define ELF32_R_SYM(i)	  ((i)>>8)
#define ELF32_R_TYPE(i)	  ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

#define ELF64_R_SYM(i)	  ((i)>>32)
#define ELF64_R_TYPE(i)	  ((i)&0xffffffffL)
#define ELF64_R_INFO(s,t) (((s)<<32)+(t)&0xffffffffL)

/* Relocation types (processor specific) - page 1-23, figure 1-22 */

#define R_386_NONE	0	/**< none */
#define R_386_32	1	/**< S + A */
#define R_386_PC32	2	/**< S + A - P */
#define R_386_GOT32	3	/**< G + A - P */
#define R_386_PLT32	4	/**< L + A - P */
#define R_386_COPY	5	/**< none */
#define R_386_GLOB_DAT	6	/**< S */
#define R_386_JMP_SLOT	7	/**< S */
#define R_386_RELATIVE	8	/**< B + A */
#define R_386_GOTOFF	9	/**< S + A - GOT */
#define R_386_GOTPC	10	/**< GOT + A - P */
#define R_386_32PLT	   11
#define R_386_TLS_TPOFF	   14		/* Offset in static TLS block */
#define R_386_TLS_IE	   15		/* Address of GOT entry for static TLS
					   block offset */
#define R_386_TLS_GOTIE	   16		/* GOT entry for static TLS block
					   offset */
#define R_386_TLS_LE	   17		/* Offset relative to static TLS
					   block */
#define R_386_TLS_GD	   18		/* Direct 32 bit for GNU version of
					   general dynamic thread local data */
#define R_386_TLS_LDM	   19		/* Direct 32 bit for GNU version of
					   local dynamic thread local data
					   in LE code */
#define R_386_16	   20
#define R_386_PC16	   21
#define R_386_8		   22
#define R_386_PC8	   23
#define R_386_TLS_GD_32	   24		/* Direct 32 bit for general dynamic
					   thread local data */
#define R_386_TLS_GD_PUSH  25		/* Tag for pushl in GD TLS code */
#define R_386_TLS_GD_CALL  26		/* Relocation for call to
					   __tls_get_addr() */
#define R_386_TLS_GD_POP   27		/* Tag for popl in GD TLS code */
#define R_386_TLS_LDM_32   28		/* Direct 32 bit for local dynamic
					   thread local data in LE code */
#define R_386_TLS_LDM_PUSH 29		/* Tag for pushl in LDM TLS code */
#define R_386_TLS_LDM_CALL 30		/* Relocation for call to
					   __tls_get_addr() in LDM code */
#define R_386_TLS_LDM_POP  31		/* Tag for popl in LDM TLS code */
#define R_386_TLS_LDO_32   32		/* Offset relative to TLS block */
#define R_386_TLS_IE_32	   33		/* GOT entry for negated static TLS
					   block offset */
#define R_386_TLS_LE_32	   34		/* Negated offset relative to static
					   TLS block */
#define R_386_TLS_DTPMOD32 35		/* ID of module containing symbol */
#define R_386_TLS_DTPOFF32 36		/* Offset in TLS block */
#define R_386_TLS_TPOFF32  37		/* Negated offset in static TLS block */
/* Keep this the last entry.  */
#define R_386_NUM	   38

/* ARM specific declarations */

/* Processor specific flags for the ELF header e_flags field.  */
#define EF_ARM_RELEXEC     0x01
#define EF_ARM_HASENTRY    0x02
#define EF_ARM_INTERWORK   0x04
#define EF_ARM_APCS_26     0x08
#define EF_ARM_APCS_FLOAT  0x10
#define EF_ARM_PIC         0x20
#define EF_ARM_ALIGN8      0x40		/* 8-bit structure alignment is in use */
#define EF_ARM_NEW_ABI     0x80
#define EF_ARM_OLD_ABI     0x100

/* Other constants defined in the ARM ELF spec. version B-01.  */
/* NB. These conflict with values defined above.  */
#define EF_ARM_SYMSARESORTED	0x04
#define EF_ARM_DYNSYMSUSESEGIDX 0x08
#define EF_ARM_MAPSYMSFIRST	0x10
#define EF_ARM_EABIMASK		0XFF000000

#define EF_ARM_EABI_VERSION(flags) ((flags) & EF_ARM_EABIMASK)
#define EF_ARM_EABI_UNKNOWN  0x00000000
#define EF_ARM_EABI_VER1     0x01000000
#define EF_ARM_EABI_VER2     0x02000000

/* Additional symbol types for Thumb */
#define STT_ARM_TFUNC      0xd

/* ARM-specific values for sh_flags */
#define SHF_ARM_ENTRYSECT  0x10000000   /* Section contains an entry point */
#define SHF_ARM_COMDEF     0x80000000   /* Section may be multiply defined
					   in the input to a link step */

/* ARM-specific program header flags */
#define PF_ARM_SB          0x10000000   /* Segment contains the location
					   addressed by the static base */

/* ARM relocs.  */
#define R_ARM_NONE		0	/* No reloc */
#define R_ARM_PC24		1	/* PC relative 26 bit branch */
#define R_ARM_ABS32		2	/* Direct 32 bit  */
#define R_ARM_REL32		3	/* PC relative 32 bit */
#define R_ARM_PC13		4
#define R_ARM_ABS16		5	/* Direct 16 bit */
#define R_ARM_ABS12		6	/* Direct 12 bit */
#define R_ARM_THM_ABS5		7
#define R_ARM_ABS8		8	/* Direct 8 bit */
#define R_ARM_SBREL32		9
#define R_ARM_THM_PC22		10
#define R_ARM_THM_PC8		11
#define R_ARM_AMP_VCALL9	12
#define R_ARM_SWI24		13
#define R_ARM_THM_SWI8		14
#define R_ARM_XPC25		15
#define R_ARM_THM_XPC22		16
#define R_ARM_COPY		20	/* Copy symbol at runtime */
#define R_ARM_GLOB_DAT		21	/* Create GOT entry */
#define R_ARM_JUMP_SLOT		22	/* Create PLT entry */
#define R_ARM_RELATIVE		23	/* Adjust by program base */
#define R_ARM_GOTOFF		24	/* 32 bit offset to GOT */
#define R_ARM_GOTPC		25	/* 32 bit PC relative offset to GOT */
#define R_ARM_GOT32		26	/* 32 bit GOT entry */
#define R_ARM_PLT32		27	/* 32 bit PLT address */
#define R_ARM_ALU_PCREL_7_0	32
#define R_ARM_ALU_PCREL_15_8	33
#define R_ARM_ALU_PCREL_23_15	34
#define R_ARM_LDR_SBREL_11_0	35
#define R_ARM_ALU_SBREL_19_12	36
#define R_ARM_ALU_SBREL_27_20	37
#define R_ARM_GNU_VTENTRY	100
#define R_ARM_GNU_VTINHERIT	101
#define R_ARM_THM_PC11		102	/* thumb unconditional branch */
#define R_ARM_THM_PC9		103	/* thumb conditional branch */
#define R_ARM_RXPC25		249
#define R_ARM_RSBREL32		250
#define R_ARM_THM_RPC22		251
#define R_ARM_RREL32		252
#define R_ARM_RABS22		253
#define R_ARM_RPC24		254
#define R_ARM_RBASE		255
/* Keep this the last entry.  */
#define R_ARM_NUM		256

/* AMD x86-64 relocations.  */
#define R_X86_64_NONE		0	/* No reloc */
#define R_X86_64_64		1	/* Direct 64 bit  */
#define R_X86_64_PC32		2	/* PC relative 32 bit signed */
#define R_X86_64_GOT32		3	/* 32 bit GOT entry */
#define R_X86_64_PLT32		4	/* 32 bit PLT address */
#define R_X86_64_COPY		5	/* Copy symbol at runtime */
#define R_X86_64_GLOB_DAT	6	/* Create GOT entry */
#define R_X86_64_JUMP_SLOT	7	/* Create PLT entry */
#define R_X86_64_RELATIVE	8	/* Adjust by program base */
#define R_X86_64_GOTPCREL	9	/* 32 bit signed PC relative
					   offset to GOT */
#define R_X86_64_32		10	/* Direct 32 bit zero extended */
#define R_X86_64_32S		11	/* Direct 32 bit sign extended */
#define R_X86_64_16		12	/* Direct 16 bit zero extended */
#define R_X86_64_PC16		13	/* 16 bit sign extended pc relative */
#define R_X86_64_8		14	/* Direct 8 bit sign extended  */
#define R_X86_64_PC8		15	/* 8 bit sign extended pc relative */
#define R_X86_64_DTPMOD64	16	/* ID of module containing symbol */
#define R_X86_64_DTPOFF64	17	/* Offset in module's TLS block */
#define R_X86_64_TPOFF64	18	/* Offset in initial TLS block */
#define R_X86_64_TLSGD		19	/* 32 bit signed PC relative offset
					   to two GOT entries for GD symbol */
#define R_X86_64_TLSLD		20	/* 32 bit signed PC relative offset
					   to two GOT entries for LD symbol */
#define R_X86_64_DTPOFF32	21	/* Offset in TLS block */
#define R_X86_64_GOTTPOFF	22	/* 32 bit signed PC relative offset
					   to GOT entry for IE symbol */
#define R_X86_64_TPOFF32	23	/* Offset in initial TLS block */

#define R_X86_64_NUM		24

/* Symbol Table Entry - page 1-17, figure 1-16 */

#define STN_UNDEF	0

/** ELF32 symbol table entry */
typedef struct {
    Elf32_Word		st_name;	/**< name of symbol (idx symstrtab) */
    Elf32_Addr		st_value;	/**< value of associated symbol */
    Elf32_Word		st_size;	/**< size of associated symbol */
    unsigned char	st_info;	/**< type and binding info */
    unsigned char	st_other;	/**< undefined */
    Elf32_Half		st_shndx;	/**< associated section header */
} Elf32_Sym;

/** ELF64 symbol table entry */
typedef struct {
    Elf64_Word		st_name;	/**< name of symbol (idx symstrtab) */
    unsigned char	st_info;	/**< type and binding info */
    unsigned char	st_other;	/**< undefined */
    Elf64_Half		st_shndx;	/**< associated section header */
    Elf64_Addr		st_value;	/**< value of associated symbol */
    Elf64_Xword		st_size;	/**< size of associated symbol */
} Elf64_Sym;

#define ELF32_ST_BIND(i)    ((i)>>4)
#define ELF32_ST_TYPE(i)    ((i)&0xf)
#define ELF32_ST_INFO(b,t)  (((b)<<4)+((t)&0xf))

#define ELF64_ST_BIND(i)    ((i)>>4)
#define ELF64_ST_TYPE(i)    ((i)&0xf)
#define ELF64_ST_INFO(b,t)  (((b)<<4)+((t)&0xf))

/* Symbol Binding - page 1-18, figure 1-17 */

#define STB_LOCAL	0	/**< not visible outside object file */
#define STB_GLOBAL	1	/**< visible to all objects beeing combined */
#define STB_WEAK	2	/**< resemble global symbols */
#define STB_LOOS	10	/**< os specific */
#define STB_HIOS	12	/**< os specific */
#define STB_LOPROC	13	/**< proc specific */
#define STB_HIPROC	15	/**< proc specific */

/* Symbol Types - page 1-19, figure 1-18 */

#define STT_NOTYPE	0	/**< symbol's type not specified */
#define STT_OBJECT	1	/**< associated with a data object */
#define STT_FUNC	2	/**< associated with a function or other code */
#define STT_SECTION	3	/**< associated with a section */
#define STT_FILE	4	/**< source file name associated with object */
#define STT_LOOS	10	/**< os specific */
#define STT_HIOS	12	/**< os specific */
#define STT_LOPROC	13	/**< proc specific */
#define STT_HIPROC	15	/**< proc specific */

enum Elf_ATs
{
  AT_NULL        = 0,
  AT_IGNORE      = 1,
  AT_EXECFD      = 2,
  AT_PHDR        = 3,
  AT_PHENT       = 4,
  AT_PHNUM       = 5,
  AT_PAGESZ      = 6,
  AT_BASE        = 7,
  AT_FLAGS       = 8,
  AT_ENTRY       = 9,
  AT_NOTELF      = 10,
  AT_UID         = 11,
  AT_EUID        = 12,
  AT_GID         = 13,
  AT_EGID        = 14,

  AT_L4_AUX      = 0xf0,
  AT_L4_ENV      = 0xf1,
};

typedef struct Elf32_Auxv
{
  Elf32_Word atype;
  Elf32_Word avalue;
} Elf32_Auxv;

typedef struct Elf64_Auxv
{
  Elf64_Word atype;
  Elf64_Word avalue;
} Elf64_Auxv;

/* Some helpers */
static inline int l4util_elf_check_magic(ElfW(Ehdr) *hdr);
static inline int l4util_elf_check_arch(ElfW(Ehdr) *hdr);
static inline ElfW(Phdr) *l4util_elf_phdr(ElfW(Ehdr) *hdr);


/* Implemeantions */
static inline
int l4util_elf_check_magic(ElfW(Ehdr) *hdr)
{
  return    hdr->e_ident[EI_MAG0] == ELFMAG0
         && hdr->e_ident[EI_MAG1] == ELFMAG1
         && hdr->e_ident[EI_MAG2] == ELFMAG2
         && hdr->e_ident[EI_MAG3] == ELFMAG3;
}

static inline
int l4util_elf_check_arch(ElfW(Ehdr) *hdr)
{
  return    hdr->e_ident[EI_CLASS] == L4_ARCH_EI_CLASS
         && hdr->e_ident[EI_DATA]  == L4_ARCH_EI_DATA
         && hdr->e_machine         == L4_ARCH_E_MACHINE;
}

static inline
ElfW(Phdr) *l4util_elf_phdr(ElfW(Ehdr) *hdr)
{
  return (ElfW(Phdr) *)((char *)hdr + hdr->e_phoff);
}
/*@}*/

#endif /* _L4_EXEC_ELF_H */
