/**
 * \file
 * \brief       ELF definition
 *
 * \date        08/18/2000
 * \author      Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *              Alexander Warg <aw11@os.inf.tu-dresden.de>
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

#pragma once

#include <l4/sys/l4int.h>

/**
 * \defgroup l4util_elf ELF binary format
 * \ingroup l4util_api
 * \brief Functions and types related to ELF binaries.
 */

/** \name ELF types
 * \ingroup l4util_elf
 */
/**@{*/
typedef l4_uint32_t     Elf32_Addr;   /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_uint32_t     Elf32_Off;    /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_uint16_t     Elf32_Half;   /**< size 2 align 2 \ingroup l4util_elf*/
typedef l4_uint32_t     Elf32_Word;   /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_int32_t      Elf32_Sword;  /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_uint64_t     Elf64_Addr;   /**< size 8 align 8 \ingroup l4util_elf*/
typedef l4_uint64_t     Elf64_Off;    /**< size 8 align 8 \ingroup l4util_elf*/
typedef l4_uint16_t     Elf64_Half;   /**< size 2 align 2 \ingroup l4util_elf*/
typedef l4_uint32_t     Elf64_Word;   /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_int32_t      Elf64_Sword;  /**< size 4 align 4 \ingroup l4util_elf*/
typedef l4_uint64_t     Elf64_Xword;  /**< size 8 align 8 \ingroup l4util_elf*/
typedef l4_int64_t      Elf64_Sxword; /**< size 8 align 8 \ingroup l4util_elf*/
/**@}*/

/**
 * Use 64 or 32 bits types depending on the target architecture.
 * \ingroup l4util_elf
 */
#if L4_MWORD_BITS == 64
# define ElfW(type)      _ElfW(Elf, 64, type)
#else
# define ElfW(type)      _ElfW(Elf, 32, type)
#endif
#define _ElfW(e,w,t)    __ElfW(e, w, _##t)
#define __ElfW(e,w,t)   e##w##t

#if defined(ARCH_x86)
# define L4_ARCH_EI_DATA      ELFDATA2LSB
# define L4_ARCH_E_MACHINE    EM_386
# define L4_ARCH_EI_CLASS     ELFCLASS32
#elif defined(ARCH_amd64)
# define L4_ARCH_EI_DATA      ELFDATA2LSB
# define L4_ARCH_E_MACHINE    EM_X86_64
# define L4_ARCH_EI_CLASS     ELFCLASS64
#elif defined(ARCH_arm)
# define L4_ARCH_EI_DATA      ELFDATA2LSB
# define L4_ARCH_E_MACHINE    EM_ARM
# define L4_ARCH_EI_CLASS     ELFCLASS32
#elif defined(ARCH_arm64)
# define L4_ARCH_EI_DATA      ELFDATA2LSB
# define L4_ARCH_E_MACHINE    EM_AARCH64
# define L4_ARCH_EI_CLASS     ELFCLASS64
#elif defined(ARCH_ppc32)
# define L4_ARCH_EI_DATA      ELFDATA2MSB
# define L4_ARCH_E_MACHINE    EM_PPC
# define L4_ARCH_EI_CLASS     ELFCLASS32
#elif defined(ARCH_sparc)
# define L4_ARCH_EI_DATA      ELFDATA2MSB
# define L4_ARCH_E_MACHINE    EM_SPARC
# define L4_ARCH_EI_CLASS     ELFCLASS32
#elif defined(ARCH_mips)
# define L4_ARCH_EI_DATA      ELFDATA2LSB
# define L4_ARCH_E_MACHINE    EM_MIPS
# ifdef __mips64
#  define L4_ARCH_EI_CLASS     ELFCLASS64
# else
#  define L4_ARCH_EI_CLASS     ELFCLASS32
# endif
#elif defined(ARCH_riscv)
# define L4_ARCH_EI_DATA      ELFDATA2LSB
# define L4_ARCH_E_MACHINE    EM_RISCV
# if __riscv_xlen == 64
#  define L4_ARCH_EI_CLASS     ELFCLASS64
# else
#  define L4_ARCH_EI_CLASS     ELFCLASS32
# endif
#else
# warning elf.h: Unsupported build architecture!
#endif


/** \addtogroup l4util_elf */

/**@{*/

enum
{
  EI_NIDENT             = 16,   /**< Number of characters. */
};

/**
 * ELF32 header.
 */
typedef struct
{
  unsigned char e_ident[EI_NIDENT]; /**< see Elf_EIs */
  Elf32_Half    e_type;         /**< type of ELF file \see Elf_ETs */
  Elf32_Half    e_machine;      /**< required architecture \see Elf_EMs */
  Elf32_Word    e_version;      /**< file version \see Elf_EVs */
  Elf32_Addr    e_entry;        /**< initial program counter */
  Elf32_Off     e_phoff;        /**< offset of program header table */
  Elf32_Off     e_shoff;        /**< offset of file header table */
  Elf32_Word    e_flags;        /**< processor-specific flags \see Elf_EF_ARM_s */
  Elf32_Half    e_ehsize;       /**< size of ELF header */
  Elf32_Half    e_phentsize;    /**< size of program header entry */
  Elf32_Half    e_phnum;        /**< number of entries in program header table */
  Elf32_Half    e_shentsize;    /**< size of section header entry */
  Elf32_Half    e_shnum;        /**< number of entries in section header table */
  Elf32_Half    e_shstrndx;     /**< section header table index of strtab */
} Elf32_Ehdr;

/**
 * ELF64 header.
 */
typedef struct
{
  unsigned char e_ident[EI_NIDENT]; /**< see Elf_EIs */
  Elf64_Half     e_type;         /**< type of ELF file \see Elf_ETs */
  Elf64_Half     e_machine;      /**< required architecture \see Elf_EMs */
  Elf64_Word     e_version;      /**< file version \see Elf_EVs */
  Elf64_Addr     e_entry;        /**< initial program counter */
  Elf64_Off      e_phoff;        /**< offset of program header table */
  Elf64_Off      e_shoff;        /**< offset of file header table */
  Elf64_Word     e_flags;        /**< processor-specific flags \see Elf_EF_ARM_s */
  Elf64_Half     e_ehsize;       /**< size of ELF header */
  Elf64_Half     e_phentsize;    /**< size of program header entry */
  Elf64_Half     e_phnum;        /**< number of entries in program header table */
  Elf64_Half     e_shentsize;    /**< size of section header entry */
  Elf64_Half     e_shnum;        /**< number of entries in section header table */
  Elf64_Half     e_shstrndx;     /**< section header table index of strtab */
} Elf64_Ehdr;

/**
 * Object file type.
 * \see Elf32_Ehdr.e_type, Elf64_Ehdr.e_type
 */
enum Elf_ETs
{
  ET_NONE               = 0,      /**< no file type */
  ET_REL                = 1,      /**< relocatable file */
  ET_EXEC               = 2,      /**< executable file */
  ET_DYN                = 3,      /**< shared object file */
  ET_CORE               = 4,      /**< core file */
  ET_LOPROC             = 0xff00, /**< processor-specific */
  ET_HIPROC             = 0xffff, /**< processor-specific */
};

/**
 * Required architecture.
 * \see Elf32_Ehdr.e_machine, Elf64_Ehdr.e_machine
 */
enum Elf_EMs
{
  EM_NONE               = 0,    /**< no machine */
  EM_M32                = 1,    /**< AT&T WE 32100 */
  EM_SPARC              = 2,    /**< SPARC */
  EM_386                = 3,    /**< Intel 80386 */
  EM_68K                = 4,    /**< Motorola 68000 */
  EM_88K                = 5,    /**< Motorola 88000 */
  EM_860                = 7,    /**< Intel 80860 */
  EM_MIPS               = 8,    /**< MIPS RS3000 big-endian */
  EM_MIPS_RS4_BE        = 10,   /**< MIPS RS4000 big-endian */
  EM_SPARC64            = 11,   /**< SPARC 64-bit */
  EM_PARISC             = 15,   /**< HP PA-RISC */
  EM_VPP500             = 17,   /**< Fujitsu VPP500 */
  EM_SPARC32PLUS        = 18,   /**< Sun's V8plus */
  EM_960                = 19,   /**< Intel 80960 */
  EM_PPC                = 20,   /**< PowerPC */
  EM_V800               = 36,   /**< NEC V800 */
  EM_FR20               = 37,   /**< Fujitsu FR20 */
  EM_RH32               = 38,   /**< TRW RH-32 */
  EM_RCE                = 39,   /**< Motorola RCE */
  EM_ARM                = 40,   /**< Advanced RISC Machines ARM */
  EM_ALPHA              = 41,   /**< Digital Alpha */
  EM_SH                 = 42,   /**< Hitachi SuperH */
  EM_SPARCV9            = 43,   /**< SPARC v9 64-bit */
  EM_TRICORE            = 44,   /**< Siemens Tricore embedded processor */
  EM_ARC                = 45,   /**< Argonaut RISC Core, Argonaut Techn Inc. */
  EM_H8_300             = 46,   /**< Hitachi H8/300 */
  EM_H8_300H            = 47,   /**< Hitachi H8/300H */
  EM_H8S                = 48,   /**< Hitachi H8/S */
  EM_H8_500             = 49,   /**< Hitachi H8/500 */
  EM_IA_64              = 50,   /**< HP/Intel IA-64 */
  EM_MIPS_X             = 51,   /**< Stanford MIPS-X */
  EM_COLDFIRE           = 52,   /**< Motorola Coldfire */
  EM_68HC12             = 53,   /**< Motorola M68HC12 */
  EM_X86_64             = 62,   /**< Advanced Micro Devices x86-64 */
  EM_PDSP               = 63,   /**< Sony DSP Processor */
  EM_FX66               = 66,   /**< Siemens FX66 microcontroller */
  EM_ST9PLUS            = 67,   /**< STMicroelectronics ST9+ 8/16 mc */
  EM_ST7                = 68,   /**< STmicroelectronics ST7 8 bit mc */
  EM_68HC16             = 69,   /**< Motorola MC68HC16 microcontroller */
  EM_68HC11             = 70,   /**< Motorola MC68HC11 microcontroller */
  EM_68HC08             = 71,   /**< Motorola MC68HC08 microcontroller */
  EM_68HC05             = 72,   /**< Motorola MC68HC05 microcontroller */
  EM_SVX                = 73,   /**< Silicon Graphics SVx */
  EM_ST19               = 74,   /**< STMicroelectronics ST19 8 bit mc */
  EM_VAX                = 75,   /**< Digital VAX */
  EM_CRIS               = 76,   /**< Axis Communications 32-bit embedded processor */
  EM_JAVELIN            = 77,   /**< Infineon Technologies 32-bit embedded processor */
  EM_FIREPATH           = 78,   /**< Element 14 64-bit DSP Processor */
  EM_ZSP                = 79,   /**< LSI Logic 16-bit DSP Processor */
  EM_MMIX               = 80,   /**< Donald Knuth's educational 64-bit processor */
  EM_HUANY              = 81,   /**< Harvard University machine-independent object files */
  EM_PRISM              = 82,   /**< SiTera Prism */
  EM_AVR                = 83,   /**< Atmel AVR 8-bit microcontroller */
  EM_FR30               = 84,   /**< Fujitsu FR30 */
  EM_D10V               = 85,   /**< Mitsubishi D10V */
  EM_D30V               = 86,   /**< Mitsubishi D30V */
  EM_V850               = 87,   /**< NEC v850 */
  EM_M32R               = 88,   /**< Mitsubishi M32R */
  EM_MN10300            = 89,   /**< Matsushita MN10300 */
  EM_MN10200            = 90,   /**< Matsushita MN10200 */
  EM_PJ                 = 91,   /**< picoJava */
  EM_OPENRISC           = 92,   /**< OpenRISC 32-bit embedded processor */
  EM_ARC_A5             = 93,   /**< ARC Cores Tangent-A5 */
  EM_XTENSA             = 94,   /**< Tensilica Xtensa Architecture */
  EM_ALTERA_NIOS2       = 113,  /**< Altera Nios II */
  EM_AARCH64            = 183,  /**< ARM AARCH64 */
  EM_TILEPRO            = 188,  /**< Tilera TILEPro */
  EM_MICROBLAZE         = 189,  /**< Xilinx MicroBlaze */
  EM_TILEGX             = 191,  /**< Tilera TILE-Gx */
  EM_RISCV              = 243,   /**< RISC-V */
  EM_NUM                = 244,
};

#if 0
#define EM_ALPHA        0x9026  /* interium value used by Linux until the
                                   committee comes up with a final number */
#define EM_S390         0xA390  /* interium value used for IBM S390 */
#endif

/** Object file version.
 * \see Elf32_Ehdr.e_version, Elf64_Ehdr.e_version */
enum Elf_EVs
{
  EV_NONE               = 0,    /**< Invalid version */
  EV_CURRENT            = 1,    /**< Current version */
};

/** Identification Indices.
 * \see Elf32_Ehdr.e_ident, Elf64_Ehdr.e_ident */
enum Elf_EIs
{
  EI_MAG0               = 0,    /**< file id 0 */
  EI_MAG1               = 1,    /**< file id 1 */
  EI_MAG2               = 2,    /**< file id 2 */
  EI_MAG3               = 3,    /**< file id 3 */
  EI_CLASS              = 4,    /**< file class */
  EI_DATA               = 5,    /**< data encoding */
  EI_VERSION            = 6,    /**< file version */
  EI_OSABI              = 7,    /**< Operating system / ABI identification */
  EI_ABIVERSION         = 8,    /**< ABI version */
  EI_PAD                = 9,    /**< start of padding bytes */
};

/** Magic number. */
enum Elf_MAGs
{
  ELFMAG0               = 0x7f, /**< e_ident[EI_MAG0] */
  ELFMAG1               = 'E',  /**< e_ident[EI_MAG1] */
  ELFMAG2               = 'L',  /**< e_ident[EI_MAG2] */
  ELFMAG3               = 'F',  /**< e_ident[EI_MAG3] */
};

/** File class or capacity. */
enum Elf_ClASSs
{
  ELFCLASSNONE          = 0,    /**< Invalid class */
  ELFCLASS32            = 1,    /**< 32-bit object */
  ELFCLASS64            = 2,    /**< 64-bit object */
  ELFCLASSNUM           = 3,    /**< Mask for 32-bit or 64-bit class. */
};

/** Data encoding. */
enum Elf_DATAs
{
  ELFDATANONE           = 0,    /**< invalid data encoding */
  ELFDATA2LSB           = 1,    /**< 0x01020304 => [ 0x04|0x03|0x02|0x01 ] */
  ELFDATA2MSB           = 2,    /**< 0x01020304 => [ 0x01|0x02|0x03|0x04 ] */
  ELFDATANUM            = 3,    /**< Mask for valid data encoding. */
};

/** Identify operating system and ABI to which the object is targeted. */
enum Elf_OSABIs
{
  ELFOSABI_NONE         = 0,    /**< UNIX System V ABI */
  ELFOSABI_SYSV         = 0,    /**< Alias.  */
  ELFOSABI_HPUX         = 1,    /**< HP-UX */
  ELFOSABI_NETBSD       = 2,    /**< NetBSD.  */
  ELFOSABI_LINUX        = 3,    /**< Linux.  */
  ELFOSABI_SOLARIS      = 6,    /**< Sun Solaris.  */
  ELFOSABI_AIX          = 7,    /**< IBM AIX.  */
  ELFOSABI_IRIX         = 8,    /**< SGI Irix.  */
  ELFOSABI_FREEBSD      = 9,    /**< FreeBSD.  */
  ELFOSABI_TRU64        = 10,   /**< Compaq TRU64 UNIX.  */
  ELFOSABI_MODESTO      = 11,   /**< Novell Modesto.  */
  ELFOSABI_OPENBSD      = 12,   /**< OpenBSD.  */
  ELFOSABI_ARM          = 97,   /**< ARM */
  ELFOSABI_STANDALONE   = 255,  /**< Standalone (embedded) application */
};

/** Special section indexes. */
enum Elf_SHNs
{
  SHN_UNDEF             = 0,            /**< undefined section header entry */
  SHN_LORESERVE         = 0xff00,       /**< lower bound of reserved indexes */
  SHN_LOPROC            = 0xff00,       /**< lower bound of proc spec entr */
  SHN_HIPROC            = 0xff1f,       /**< upper bound of proc spec entr */
  SHN_ABS               = 0xfff1,       /**< absolute values for ref */
  SHN_COMMON            = 0xfff2,       /**< common symbols */
  SHN_HIRESERVE         = 0xffff,       /**< upper bound of reserved indexes */
};

/** ELF32 section header. */
typedef struct
{
  Elf32_Word            sh_name;        /**< name of sect (idx into strtab) */
  Elf32_Word            sh_type;        /**< section's type \see Elf_SHTs */
  Elf32_Word            sh_flags;       /**< section's flags \see Elf_SHFs */
  Elf32_Addr            sh_addr;        /**< memory address of section */
  Elf32_Off             sh_offset;      /**< file offset of section */
  Elf32_Word            sh_size;        /**< file size of section */
  Elf32_Word            sh_link;        /**< idx to associated header section */
  Elf32_Word            sh_info;        /**< extra info of header section */
  Elf32_Word            sh_addralign;   /**< address alignment constraints */
  Elf32_Word            sh_entsize;     /**< size of entry if sect is table */
} Elf32_Shdr;

/** ELF64 section header. */
typedef struct
{
  Elf64_Word            sh_name;        /**< name of sect (idx into strtab) */
  Elf64_Word            sh_type;        /**< section's type \see Elf_SHTs */
  Elf64_Xword           sh_flags;       /**< section's flags \see Elf_SHFs */
  Elf64_Addr            sh_addr;        /**< memory address of section */
  Elf64_Off             sh_offset;      /**< file offset of section */
  Elf64_Xword           sh_size;        /**< file size of section */
  Elf64_Word            sh_link;        /**< idx to associated header section */
  Elf64_Word            sh_info;        /**< extra info of header section */
  Elf64_Xword           sh_addralign;   /**< address alignment constraints */
  Elf64_Xword           sh_entsize;     /**< size of entry if sect is table */
} Elf64_Shdr;

/** Section type. */
enum Elf_SHTs
{
  SHT_NULL              = 0,    /**< inactive section header */
  SHT_PROGBITS          = 1,    /**< information defined by program */
  SHT_SYMTAB            = 2,    /**< symbol table */
  SHT_STRTAB            = 3,    /**< string table */
  SHT_RELA              = 4,    /**< reloc entries w/ explicit addens */
  SHT_HASH              = 5,    /**< symbol hash table */
  SHT_DYNAMIC           = 6,    /**< information for dynamic linking */
  SHT_NOTE              = 7,    /**< information that marks the file */
  SHT_NOBITS            = 8,    /**< occupies no space in the file */
  SHT_REL               = 9,    /**< reloc entries w/o explicit addens */
  SHT_SHLIB             = 10,   /**< reserved + unspecified semantics */
  SHT_DYNSYM            = 11,   /**< symbol table (dynamic */
  SHT_INIT_ARRAY        = 14,   /**< Array of constructors */
  SHT_FINI_ARRAY        = 15,   /**< Array of destructors */
  SHT_PREINIT_ARRAY     = 16,   /**< Array of pre-constructors */
  SHT_GROUP             = 17,   /**< Section group */
  SHT_SYMTAB_SHNDX      = 18,   /**< Extended section indices */
  SHT_NUM               = 19,   /**< Number of defined types.  */
  SHT_LOOS              = 0x60000000, /**< Start OS-specific. */
  SHT_HIOS              = 0x6fffffff, /**< End OS-specific. */
  SHT_LOPROC            = 0x70000000, /**< Start processor-specific. */
  SHT_HIPROC            = 0x7fffffff, /**< End processor-specific. */
  SHT_LOUSER            = 0x80000000, /**< Start application-specific. */
  SHT_HIUSER            = 0xffffffff, /**< End application-specific. */
};

/** Section attribute flags. */
enum Elf_SHFs
{
  SHF_WRITE             = 0x1,  /**< writeable during execution */
  SHF_ALLOC             = 0x2,  /**< section occupies virt memory */
  SHF_EXECINSTR         = 0x4,  /**< code section */
  SHF_MERGE             = 0x10, /**< Might be merged */
  SHF_STRINGS           = 0x20, /**< Contains nul-terminated strings */
  SHF_INFO_LINK         = 0x40, /**< `sh_info' contains SHT index */
  SHF_LINK_ORDER        = 0x80, /**< Preserve order after combining */
  SHF_OS_NONCONFORMING  = 0x100, /**< Non-standard OS-specific handling
                                      required */
  SHF_GROUP             = 0x200, /**< Section is member of a group.  */
  SHF_TLS               = 0x400, /**< Section hold thread-local data.  */
  SHF_MASKOS            = 0x0ff00000, /**< OS-specific.  */
  SHF_MASKPROC          = 0xf0000000, /**< processor-specific mask */
};


/** ELF32 program header. */
typedef struct
{
  Elf32_Word    p_type;         /**< type of program section \see Elf_PTs */
  Elf32_Off     p_offset;       /**< file offset of program section */
  Elf32_Addr    p_vaddr;        /**< memory address of prog section */
  Elf32_Addr    p_paddr;        /**< physical address (ignored) */
  Elf32_Word    p_filesz;       /**< file size of program section */
  Elf32_Word    p_memsz;        /**< memory size of program section */
  Elf32_Word    p_flags;        /**< flags \see Elf_PFs */
  Elf32_Word    p_align;        /**< alignment of section */
} Elf32_Phdr;

/** ELF64 program header. */
typedef struct
{
  Elf64_Word    p_type;         /**< type of program section \see Elf_PTs */
  Elf64_Word    p_flags;        /**< flags \see Elf_PFs */
  Elf64_Off     p_offset;       /**< file offset of program section */
  Elf64_Addr    p_vaddr;        /**< memory address of prog section */
  Elf64_Addr    p_paddr;        /**< physical address (ignored) */
  Elf64_Xword   p_filesz;       /**< file size of program section */
  Elf64_Xword   p_memsz;        /**< memory size of program section */
  Elf64_Xword   p_align;        /**< alignment of section */
} Elf64_Phdr;

/** Segment types. */
enum Elf_PTs
{
  PT_NULL               = 0,    /**< array is unused */
  PT_LOAD               = 1,    /**< loadable */
  PT_DYNAMIC            = 2,    /**< dynamic linking information */
  PT_INTERP             = 3,    /**< path to interpreter */
  PT_NOTE               = 4,    /**< auxiliary information */
  PT_SHLIB              = 5,    /**< reserved */
  PT_PHDR               = 6,    /**< location of the pht itself */
  PT_TLS                = 7,    /**< Thread-local storage segment */
  PT_NUM                = 8,    /**< Number of defined types */
  PT_LOOS               = 0x60000000, /**< OS-specific */
  PT_HIOS               = 0x6fffffff, /**< OS-specific */
  PT_LOPROC             = 0x70000000, /**< processor-specific */
  PT_HIPROC             = 0x7fffffff, /**< processor-specific */

  PT_GNU_EH_FRAME       = PT_LOOS + 0x474e550, /**< EH frame information. */
  PT_GNU_STACK          = PT_LOOS + 0x474e551, /**< Flags for stack. */
  PT_GNU_RELRO          = PT_LOOS + 0x474e552, /**< Read only after reloc. */

  PT_L4_STACK           = PT_LOOS + 0x12, /**< Address of the stack. */
  PT_L4_KIP             = PT_LOOS + 0x13, /**< Address of the KIP. */
  PT_L4_AUX             = PT_LOOS + 0x14, /**< Address of the AUX structures. */
};

/** Segment permissions. */
enum ELF_PFs
{
  PF_X                  = 0x1,  /**< Executable */
  PF_W                  = 0x2,  /**< Write */
  PF_R                  = 0x4,  /**< Read */
  PF_MASKOS             = 0x0ff00000, /**< OS-specific */
  PF_MASKPROC           = 0x7fffffff, /**< Processor-specific */
};

/** Legal values for note segment descriptor types for core files. */
enum Elf_NTs_core
{
  NT_PRSTATUS           = 1,    /**< Contains copy of prstatus struct */
  NT_FPREGSET           = 2,    /**< Contains copy of fpregset struct */
  NT_PRPSINFO           = 3,    /**< Contains copy of prpsinfo struct */
  NT_PRXREG             = 4,    /**< Contains copy of prxregset struct */
  NT_TASKSTRUCT         = 4,    /**< Contains copy of task structure */
  NT_PLATFORM           = 5,    /**< String from sysinfo(SI_PLATFORM) */
  NT_AUXV               = 6,    /**< Contains copy of auxv array */
  NT_GWINDOWS           = 7,    /**< Contains copy of gwindows struct */
  NT_ASRS               = 8,    /**< Contains copy of asrset struct */
  NT_PSTATUS            = 10,   /**< Contains copy of pstatus struct */
  NT_PSINFO             = 13,   /**< Contains copy of psinfo struct */
  NT_PRCRED             = 14,   /**< Contains copy of prcred struct */
  NT_UTSNAME            = 15,   /**< Contains copy of utsname struct */
  NT_LWPSTATUS          = 16,   /**< Contains copy of lwpstatus struct */
  NT_LWPSINFO           = 17,   /**< Contains copy of lwpinfo struct */
  NT_PRFPXREG           = 20,   /**< Contains copy of fprxregset struct*/
};

/** Legal values for the note segment descriptor types for object files.  */
enum Elf_NTs_obj
{
  NT_VERSION            = 1,       /**< Contains a version string. */
};

/** ELF32 dynamic entry. */
typedef struct
{
  Elf32_Sword   d_tag;  /**< \see Elf_DTs */
  union
  {
    Elf32_Word  d_val;  /**< integer values with various interpret. */
    Elf32_Addr  d_ptr;  /**< program virtual addresses */
  } d_un;
} Elf32_Dyn;

/** ELF64 dynamic entry. */
typedef struct
{
  Elf64_Sxword  d_tag;  /**< \see Elf_DTs */
  union
  {
    Elf64_Xword d_val;  /**< integer values with various interpret. */
    Elf64_Addr  d_ptr;  /**< program virtual addresses */
  } d_un;
} Elf64_Dyn;

/** Dynamic Array Tags. \see Elf32_Dyn.d_tag, Elf64_Dyn.d_tag. */
enum Elf_DTs
{
  DT_NULL               = 0,    /**< end of _DYNAMIC array */
  DT_NEEDED             = 1,    /**< name of a needed library */
  DT_PLTRELSZ           = 2,    /**< total size of relocation entry */
  DT_PLTGOT             = 3,    /**< address assoc with prog link table */
  DT_HASH               = 4,    /**< address of symbol hash table */
  DT_STRTAB             = 5,    /**< address of string table */
  DT_SYMTAB             = 6,    /**< address of symbol table */
  DT_RELA               = 7,    /**< address of relocation table */
  DT_RELASZ             = 8,    /**< total size of relocation table */
  DT_RELAENT            = 9,    /**< size of DT_RELA relocation entry */
  DT_STRSZ              = 10,   /**< size of the string table */
  DT_SYMENT             = 11,   /**< size of a symbol table entry */
  DT_INIT               = 12,   /**< address of initialization function */
  DT_FINI               = 13,   /**< address of termination function */
  DT_SONAME             = 14,   /**< name of the shared object */
  DT_RPATH              = 15,   /**< search library path */
  DT_SYMBOLIC           = 16,   /**< alter symbol resolution algorithm */
  DT_REL                = 17,   /**< address of relocation table */
  DT_RELSZ              = 18,   /**< total size of DT_REL relocation table */
  DT_RELENT             = 19,   /**< size of the DT_REL relocation entry */
  DT_PTRREL             = 20,   /**< type of relocation entry */
  DT_DEBUG              = 21,   /**< for debugging purposes */
  DT_TEXTREL            = 22,   /**< at least on entry changes r/o section */
  DT_JMPREL             = 23,   /**< address of relocation entries */
  DT_BIND_NOW           = 24,   /**< Process relocations of object */
  DT_INIT_ARRAY         = 25,   /**< Array with addresses of init fct */
  DT_FINI_ARRAY         = 26,   /**< Array with addresses of fini fct */
  DT_INIT_ARRAYSZ       = 27,   /**< Size in bytes of DT_INIT_ARRAY */
  DT_FINI_ARRAYSZ       = 28,   /**< Size in bytes of DT_FINI_ARRAY */
  DT_RUNPATH            = 29,   /**< Library search path */
  DT_FLAGS              = 30,   /**< Flags for the object being loaded */
  DT_ENCODING           = 32,   /**< Start of encoded range */
  DT_PREINIT_ARRAY      = 32,   /**< Array with addresses of preinit fct*/
  DT_PREINIT_ARRAYSZ    = 33,   /**< size in bytes of DT_PREINIT_ARRAY */
  DT_NUM                = 34,   /**< Number used */
  DT_LOOS               = 0x6000000d, /**< Start of OS-specific */
  DT_HIOS               = 0x6ffff000, /**< End of OS-specific */
  DT_LOPROC             = 0x70000000, /**< processor-specific */
  DT_HIPROC             = 0x7fffffff, /**< processor-specific */
};

/**
 * Values of Elf32_Dyn.d_un.d_val, Elf64_Dyn.d_un.d_val in the DT_FLAGS entry.
 */
enum Elf_DFs
{
  DF_ORIGIN             = 0x00000001,   /**< Object may use DF_ORIGIN */
  DF_SYMBOLIC           = 0x00000002,   /**< Symbol resolutions starts here */
  DF_TEXTREL            = 0x00000004,   /**< Object contains text relocations */
  DF_BIND_NOW           = 0x00000008,   /**< No lazy binding for this object */
  DF_STATIC_TLS         = 0x00000010,   /**< Module uses the static TLS model */
};

/**
 * State flags selectable in the Elf32_Dyn.d_un.d_val / Elf64_Dyn.d_un.d_val
 * element of the DT_FLAGS_1 entry in the dynamic section.
 */
enum Elf_DF_1s
{
  DF_1_NOW              = 0x00000001,   /**< Set RTLD_NOW for this object.  */
  DF_1_GLOBAL           = 0x00000002,   /**< Set RTLD_GLOBAL for this object.  */
  DF_1_GROUP            = 0x00000004,   /**< Set RTLD_GROUP for this object.  */
  DF_1_NODELETE         = 0x00000008,   /**< Set RTLD_NODELETE for this object.*/
  DF_1_LOADFLTR         = 0x00000010,   /**< Trigger filtee loading at runtime.*/
  DF_1_INITFIRST        = 0x00000020,   /**< Set RTLD_INITFIRST for this object*/
  DF_1_NOOPEN           = 0x00000040,   /**< Set RTLD_NOOPEN for this object.  */
  DF_1_ORIGIN           = 0x00000080,   /**< $ORIGIN must be handled.  */
  DF_1_DIRECT           = 0x00000100,   /**< Direct binding enabled.  */
  DF_1_TRANS            = 0x00000200,
  DF_1_INTERPOSE        = 0x00000400,   /**< Object is used to interpose.  */
  DF_1_NODEFLIB         = 0x00000800,   /**< Ignore default lib search path.  */
  DF_1_NODUMP           = 0x00001000,   /**< Object can't be dldump'ed.  */
  DF_1_CONFALT          = 0x00002000,   /**< Configuration alternative created.*/
  DF_1_ENDFILTEE        = 0x00004000,   /**< Filtee terminates filters search. */
  DF_1_DISPRELDNE       = 0x00008000,   /**< Disp reloc applied at build time. */
  DF_1_DISPRELPND       = 0x00010000,   /**< Disp reloc applied at run-time.  */
};

/** Flags for the feature selection in DT_FEATURE_1. */
enum Elf_DTF_1s
{
  DTF_1_PARINIT         = 0x00000001,
  DTF_1_CONFEXP         = 0x00000002,
};

/** Flags in the DT_POSFLAG_1 entry effecting only the next DT_* entry. */
enum Elf_DF_P1s
{
  DF_P1_LAZYLOAD        = 0x00000001,   /**< Lazyload following object.  */
  DF_P1_GROUPPERM       = 0x00000002,   /**< Symbols from next object are not
                                             generally available.  */
};

/** ELF32 relocation entry w/o addend. */
typedef struct
{
  Elf32_Addr          r_offset;
  Elf32_Word          r_info;
} Elf32_Rel;

/** ELF32 relocation entry w/ addend. */
typedef struct
{
  Elf32_Addr          r_offset;
  Elf32_Word          r_info;
  Elf32_Sword         r_addend;
} Elf32_Rela;

/** ELF64 relocation entry w/o addend. */
typedef struct
{
  Elf64_Addr          r_offset;
  Elf64_Xword         r_info;
} Elf64_Rel;

/** ELF64 relocation entry w/ addend. */
typedef struct
{
  Elf64_Addr          r_offset;
  Elf64_Xword         r_info;
  Elf64_Sxword        r_addend;
} Elf64_Rela;

/** Symbol table index. */
#define ELF32_R_SYM(i)    ((i)>>8)
/** \see Elf_R_386s. */
#define ELF32_R_TYPE(i)   ((unsigned char)(i))
/** Create info from symbol table index + type. */
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

/** Symbol table index. */
#define ELF64_R_SYM(i)    ((i)>>32)

/** \see Elf_R_386s. */
#define ELF64_R_TYPE(i)   ((i)&0xffffffffL)

/** Create info from symbol table index + type. */
#define ELF64_R_INFO(s,t) (((s)<<32)+(t)&0xffffffffL)

/** Relocation types (processor specific). */
enum Elf_R_386_s
{
 R_386_NONE             = 0,    /**< none */
 R_386_32               = 1,    /**< S + A */
 R_386_PC32             = 2,    /**< S + A - P */
 R_386_GOT32            = 3,    /**< G + A - P */
 R_386_PLT32            = 4,    /**< L + A - P */
 R_386_COPY             = 5,    /**< none */
 R_386_GLOB_DAT         = 6,    /**< S */
 R_386_JMP_SLOT         = 7,    /**< S */
 R_386_RELATIVE         = 8,    /**< B + A */
 R_386_GOTOFF           = 9,    /**< S + A - GOT */
 R_386_GOTPC            = 10,   /**< GOT + A - P */
 R_386_32PLT            = 11,
 R_386_TLS_TPOFF        = 14,   /**< Offset in static TLS block */
 R_386_TLS_IE           = 15,   /**< Address of GOT entry for static TLS
                                     block offset */
 R_386_TLS_GOTIE        = 16,   /**< GOT entry for static TLS block offset */
 R_386_TLS_LE           = 17,   /**< Offset relative to static TLS block */
 R_386_TLS_GD           = 18,   /**< Direct 32 bit for GNU version of
                                     general dynamic thread local data */
 R_386_TLS_LDM          = 19,   /**< Direct 32 bit for GNU version of local
                                     dynamic thread local data in LE code */
 R_386_16               = 20,
 R_386_PC16             = 21,
 R_386_8                = 22,
 R_386_PC8              = 23,
 R_386_TLS_GD_32        = 24,   /**< Direct 32 bit for general dynamic thread
                                     local data */
 R_386_TLS_GD_PUSH      = 25,   /**< Tag for pushl in GD TLS code */
 R_386_TLS_GD_CALL      = 26,   /**< Relocation for call to
                                     __tls_get_addr() */
 R_386_TLS_GD_POP       = 27,   /**< Tag for popl in GD TLS code */
 R_386_TLS_LDM_32       = 28,   /**< Direct 32 bit for local dynamic thread
                                     local data in LE code */
 R_386_TLS_LDM_PUSH     = 29,   /**< Tag for pushl in LDM TLS code */
 R_386_TLS_LDM_CALL     = 30,   /**< Relocation for call to
                                     __tls_get_addr() in LDM code */
 R_386_TLS_LDM_POP      = 31,   /**< Tag for popl in LDM TLS code */
 R_386_TLS_LDO_32       = 32,   /**< Offset relative to TLS block */
 R_386_TLS_IE_32        = 33,   /**< GOT entry for negated static TLS block
                                     offset */
 R_386_TLS_LE_32        = 34,   /**< Negated offset relative to static
                                     TLS block */
 R_386_TLS_DTPMOD32     = 35,   /**< ID of module containing symbol */
 R_386_TLS_DTPOFF32     = 36,   /**< Offset in TLS block */
 R_386_TLS_TPOFF32      = 37,   /**< Negated offset in static TLS block */
 R_386_NUM              = 38,   /**< Keep this the last entry. */
};

/** ARM specific declarations. */

/** Processor specific flags for the ELF header e_flags field.  */
enum Elf_EF_ARM_s
{
  EF_ARM_RELEXEC        = 0x01,
  EF_ARM_HASENTRY       = 0x02,
  EF_ARM_INTERWORK      = 0x04,
  EF_ARM_APCS_26        = 0x08,
  EF_ARM_APCS_FLOAT     = 0x10,
  EF_ARM_PIC            = 0x20,
  EF_ARM_ALIGN8         = 0x40, /**< 8-bit structure alignment is in use */
  EF_ARM_NEW_ABI        = 0x80,
  EF_ARM_OLD_ABI        = 0x100,

/* Other constants defined in the ARM ELF spec. version B-01.  */
/* NB. These conflict with values defined above.  */
  EF_ARM_SYMSARESORTED  = 0x04,
  EF_ARM_DYNSYMSUSESEGIDX = 0x08,
  EF_ARM_MAPSYMSFIRST   = 0x10,
  EF_ARM_EABIMASK       = 0XFF000000,

#define EF_ARM_EABI_VERSION(flags) ((flags) & EF_ARM_EABIMASK)
  EF_ARM_EABI_UNKNOWN   = 0x00000000,
  EF_ARM_EABI_VER1      = 0x01000000,
  EF_ARM_EABI_VER2      = 0x02000000,
};

/** Additional symbol types for Thumb. */
enum Elf_STT_ARM_s
{
  STT_ARM_TFUNC         = 0xd,
};

/** ARM-specific values for Elf32_Shdr.sh_flags / Elf64_Shdr.sh_flags. */
enum Elf_SHF_s_ARM
{
  SHF_ARM_ENTRYSECT     = 0x10000000,   /**< Section contains an entry point */
  SHF_ARM_COMDEF        = 0x80000000,   /**< Section may be multiply defined
                                             in the input to a link step */
};

/** ARM-specific program header flags. */
enum Elf_ARM_SBs
{
  PF_ARM_SB             = 0x10000000,   /**< Segment contains the location
                                             addressed by the static base */
};

/** ARM relocations. */
enum Elf_R_ARM_s
{
  R_ARM_NONE            = 0,    /**< No reloc */
  R_ARM_PC24            = 1,    /**< PC relative 26 bit branch */
  R_ARM_ABS32           = 2,    /**< Direct 32 bit  */
  R_ARM_REL32           = 3,    /**< PC relative 32 bit */
  R_ARM_PC13            = 4,
  R_ARM_ABS16           = 5,    /**< Direct 16 bit */
  R_ARM_ABS12           = 6,    /**< Direct 12 bit */
  R_ARM_THM_ABS5        = 7,
  R_ARM_ABS8            = 8,    /**< Direct 8 bit */
  R_ARM_SBREL32         = 9,
  R_ARM_THM_PC22        = 10,
  R_ARM_THM_PC8         = 11,
  R_ARM_AMP_VCALL9      = 12,
  R_ARM_SWI24           = 13,
  R_ARM_THM_SWI8        = 14,
  R_ARM_XPC25           = 15,
  R_ARM_THM_XPC22       = 16,
  R_ARM_COPY            = 20,   /**< Copy symbol at runtime */
  R_ARM_GLOB_DAT        = 21,   /**< Create GOT entry */
  R_ARM_JUMP_SLOT       = 22,   /**< Create PLT entry */
  R_ARM_RELATIVE        = 23,   /**< Adjust by program base */
  R_ARM_GOTOFF          = 24,   /**< 32 bit offset to GOT */
  R_ARM_GOTPC           = 25,   /**< 32 bit PC relative offset to GOT */
  R_ARM_GOT32           = 26,   /**< 32 bit GOT entry */
  R_ARM_PLT32           = 27,   /**< 32 bit PLT address */
  R_ARM_ALU_PCREL_7_0   = 32,
  R_ARM_ALU_PCREL_15_8  = 33,
  R_ARM_ALU_PCREL_23_15 = 34,
  R_ARM_LDR_SBREL_11_0  = 35,
  R_ARM_ALU_SBREL_19_12 = 36,
  R_ARM_ALU_SBREL_27_20 = 37,
  R_ARM_GNU_VTENTRY     = 100,
  R_ARM_GNU_VTINHERIT   = 101,
  R_ARM_THM_PC11        = 102,  /**< thumb unconditional branch */
  R_ARM_THM_PC9         = 103,  /**< thumb conditional branch */
  R_ARM_RXPC25          = 249,
  R_ARM_RSBREL32        = 250,
  R_ARM_THM_RPC22       = 251,
  R_ARM_RREL32          = 252,
  R_ARM_RABS22          = 253,
  R_ARM_RPC24           = 254,
  R_ARM_RBASE           = 255,
  R_ARM_NUM             = 256,  /**< Keep this the last entry. */
};

/** AARCH64 relocations. */
enum Elf_R_AARCH64_s
{
  R_AARCH64_NONE        = 0,    /**< No reloc */
  R_AARCH64_RELATIVE    = 1027,
};

/** AMD x86-64 relocations. */
enum Elf_R_X86_64_s
{
  R_X86_64_NONE         = 0,    /**< No reloc */
  R_X86_64_64           = 1,    /**< Direct 64 bit  */
  R_X86_64_PC32         = 2,    /**< PC relative 32 bit signed */
  R_X86_64_GOT32        = 3,    /**< 32 bit GOT entry */
  R_X86_64_PLT32        = 4,    /**< 32 bit PLT address */
  R_X86_64_COPY         = 5,    /**< Copy symbol at runtime */
  R_X86_64_GLOB_DAT     = 6,    /**< Create GOT entry */
  R_X86_64_JUMP_SLOT    = 7,    /**< Create PLT entry */
  R_X86_64_RELATIVE     = 8,    /**< Adjust by program base */
  R_X86_64_GOTPCREL     = 9,    /**< 32 bit signed PC relative offset to GOT */
  R_X86_64_32           = 10,   /**< Direct 32 bit zero extended */
  R_X86_64_32S          = 11,   /**< Direct 32 bit sign extended */
  R_X86_64_16           = 12,   /**< Direct 16 bit zero extended */
  R_X86_64_PC16         = 13,   /**< 16 bit sign extended pc relative */
  R_X86_64_8            = 14,   /**< Direct 8 bit sign extended  */
  R_X86_64_PC8          = 15,   /**< 8 bit sign extended pc relative */
  R_X86_64_DTPMOD64     = 16,   /**< ID of module containing symbol */
  R_X86_64_DTPOFF64     = 17,   /**< Offset in module's TLS block */
  R_X86_64_TPOFF64      = 18,   /**< Offset in initial TLS block */
  R_X86_64_TLSGD        = 19,   /**< 32 bit signed PC relative offset to two
                                     GOT entries for GD symbol */
  R_X86_64_TLSLD        = 20,   /**< 32 bit signed PC relative offset to two
                                     GOT entries for LD symbol */
  R_X86_64_DTPOFF32     = 21,   /**< Offset in TLS block */
  R_X86_64_GOTTPOFF     = 22,   /**< 32 bit signed PC relative offset to GOT
                                     entry for IE symbol */
  R_X86_64_TPOFF32      = 23,   /**< Offset in initial TLS block */
  R_X86_64_NUM          = 24,
};

/** Symbol Table Entry. */
enum Elf_STNs
{
  STN_UNDEF             = 0,
};

/** ELF32 symbol table entry. */
typedef struct
{
  Elf32_Word            st_name;        /**< name of symbol (idx symstrtab) */
  Elf32_Addr            st_value;       /**< value of associated symbol */
  Elf32_Word            st_size;        /**< size of associated symbol */
  unsigned char         st_info;        /**< type and binding info */
  unsigned char         st_other;       /**< undefined */
  Elf32_Half            st_shndx;       /**< associated section header */
} Elf32_Sym;

/** ELF64 symbol table entry. */
typedef struct
{
  Elf64_Word            st_name;        /**< name of symbol (idx symstrtab) */
  unsigned char         st_info;        /**< type and binding info */
  unsigned char         st_other;       /**< undefined */
  Elf64_Half            st_shndx;       /**< associated section header */
  Elf64_Addr            st_value;       /**< value of associated symbol */
  Elf64_Xword           st_size;        /**< size of associated symbol */
} Elf64_Sym;

/** \see Elf_STBs. */
#define ELF32_ST_BIND(i)    ((i)>>4)

/** \see Elf_STTs. */
#define ELF32_ST_TYPE(i)    ((i)&0xf)

/** Make info from bind + type. */
#define ELF32_ST_INFO(b,t)  (((b)<<4)+((t)&0xf))

/** \see Elf_STBs */
#define ELF64_ST_BIND(i)    ((i)>>4)

/** \see Elf_STTs */
#define ELF64_ST_TYPE(i)    ((i)&0xf)

/** Make info from bind + type. */
#define ELF64_ST_INFO(b,t)  (((b)<<4)+((t)&0xf))

/** Symbol Binding.
 * \see ELF32_ST_BIND, ELF64_ST_BIND */
enum Elf_STBs
{
  STB_LOCAL     = 0,    /**< not visible outside object file */
  STB_GLOBAL    = 1,    /**< visible to all objects being combined */
  STB_WEAK      = 2,    /**< resemble global symbols */
  STB_LOOS      = 10,   /**< OS-specific */
  STB_HIOS      = 12,   /**< OS-specific */
  STB_LOPROC    = 13,   /**< Processor-specific */
  STB_HIPROC    = 15,   /**< Processor-specific */
};

/** Symbol Types.
 * \see ELF32_ST_TYPE, ELF64_ST_TYPE */
enum Elf_STTs
{
  STT_NOTYPE    = 0,    /**< symbol's type not specified */
  STT_OBJECT    = 1,    /**< associated with a data object */
  STT_FUNC      = 2,    /**< associated with a function or other code */
  STT_SECTION   = 3,    /**< associated with a section */
  STT_FILE      = 4,    /**< source file name associated with object */
  STT_LOOS      = 10,   /**< OS-specific */
  STT_HIOS      = 12,   /**< OS-specific */
  STT_LOPROC    = 13,   /**< processor-specific */
  STT_HIPROC    = 15,   /**< processor-specific */
};

/** Legal values for Elf32_Auxv.atype / Elf64_Auxv.atype. */
enum Elf_ATs
{
  AT_NULL        = 0,   /**< End of vector */
  AT_IGNORE      = 1,   /**< Entry should be ignored */
  AT_EXECFD      = 2,   /**< File descriptor of program */
  AT_PHDR        = 3,   /**< Program headers for program */
  AT_PHENT       = 4,   /**< Size of program header entry */
  AT_PHNUM       = 5,   /**< Number of program headers */
  AT_PAGESZ      = 6,   /**< System page size */
  AT_BASE        = 7,   /**< Base address of interpreter */
  AT_FLAGS       = 8,   /**< Flags */
  AT_ENTRY       = 9,   /**< Entry point of program */
  AT_NOTELF      = 10,  /**< Program is not ELF */
  AT_UID         = 11,  /**< Real UID */
  AT_EUID        = 12,  /**< Effective UID */
  AT_GID         = 13,  /**< Real GID */
  AT_EGID        = 14,  /**< Effective GID */

  AT_L4_AUX      = 0xf0, /**< L4Re AUX section */
  AT_L4_ENV      = 0xf1, /**< L4Re ENV section */
};

/** Auxiliary vector (32-bit). */
typedef struct Elf32_Auxv
{
  Elf32_Word atype;     /**< \see Elf_ATs */
  Elf32_Word avalue;
} Elf32_Auxv;

/** Auxiliary vector (64-bit). */
typedef struct Elf64_Auxv
{
  Elf64_Word atype;     /**< \see Elf_ATs */
  Elf64_Word avalue;
} Elf64_Auxv;

/**
 * Check ELF magic in ELF header.
 *
 * \param hdr  ELF header. \see Elf32_Ehdr, Elf64_Ehdr
 * \retval true valid ELF header found.
 * \retval false no valid ELF header found.
 */
static inline int l4util_elf_check_magic(ElfW(Ehdr) const *hdr);

/**
 * Check ELF architecture.
 *
 * \param hdr  ELF header. \see Elf32_Ehdr, Elf64_Ehdr
 * \retval true ELF architecture matches.
 * \retval false ELF architecture doesn't match.
 */
static inline int l4util_elf_check_arch(ElfW(Ehdr) const *hdr);

/**
 * Return ELF program sections.
 *
 * \param hdr  ELF header. \see Elf32_Ehdr, Elf64_Ehdr
 * \return Pointer to ELF program sections.
 */
static inline ElfW(Phdr) *l4util_elf_phdr(ElfW(Ehdr) const *hdr);


/* Implementations */

static inline
int l4util_elf_check_magic(ElfW(Ehdr) const *hdr)
{
  return    hdr->e_ident[EI_MAG0] == ELFMAG0
         && hdr->e_ident[EI_MAG1] == ELFMAG1
         && hdr->e_ident[EI_MAG2] == ELFMAG2
         && hdr->e_ident[EI_MAG3] == ELFMAG3;
}

static inline
int l4util_elf_check_arch(ElfW(Ehdr) const *hdr)
{
  return    hdr->e_ident[EI_CLASS] == L4_ARCH_EI_CLASS
         && hdr->e_ident[EI_DATA]  == L4_ARCH_EI_DATA
         && hdr->e_machine         == L4_ARCH_E_MACHINE;
}

static inline
ElfW(Phdr) *l4util_elf_phdr(ElfW(Ehdr) const *hdr)
{
  return (ElfW(Phdr) *)((char *)hdr + hdr->e_phoff);
}
/**@}*/
