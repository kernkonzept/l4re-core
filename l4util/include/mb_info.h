/**
 * \file
 *
 * \brief	Multiboot info structure as defined by GRUB */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Frank Mehnert <fm3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef L4UTIL_MB_INFO_H
#define L4UTIL_MB_INFO_H

#ifndef __ASSEMBLY__

#include <l4/sys/l4int.h>

/**
 * \anchor struct_l4util_mod_list
 *  The structure type "mod_list" is used by the 
 *  \ref struct_l4util_mb_info "multiboot_info" structure.
 */

typedef struct
{
  l4_uint32_t mod_start;	/**< Starting address of module in memory. */
  l4_uint32_t mod_end;		/**< End address of module in memory. */
  l4_uint32_t cmdline;		/**< Module command line */
  l4_uint32_t pad;		/**< padding to take it to 16 bytes */
} l4util_mb_mod_t;


/**
 *  INT-15, AX=E820 style "AddressRangeDescriptor"
 *  ...with a "size" parameter on the front which is the structure size - 4,
 *  pointing to the next one, up until the full buffer length of the memory
 *  map has been reached.
 */

typedef struct __attribute__((packed))
{
  l4_uint32_t struct_size;	/** <Size of structure */
  l4_uint64_t addr;		/** <Start address */
  l4_uint64_t size;		/** <Size of memory range */
  l4_uint32_t type;		/** <type of memory range */
  /* unspecified optional padding... */
} l4util_mb_addr_range_t;

#define l4util_mb_for_each_mmap_entry(i, mbi) \
  for (i = (l4util_mb_addr_range_t *)(unsigned long)mbi->mmap_addr; \
       (unsigned long)i < (unsigned long)mbi->mmap_addr + mbi->mmap_length; \
       i = (l4util_mb_addr_range_t *)((unsigned long)i + mmap->struct_size + sizeof (mmap->struct_size)))

/** usable memory "Type", all others are reserved.  */
#define MB_ARD_MEMORY		1

/**
 * Address Range Types (ART) from "Advanced Configuration and Power Interface
 * Specification" Rev3.0a (p. 390). Other values are undefined.
 */
#define MB_ART_MEMORY   1  /**< available, usable RAM */
#define MB_ART_RESERVED 2  /**< in use or reserved by system */
#define MB_ART_ACPI     3  /**< ACPI Reclaim Memory (RAM that contains
			        ACPI tables) */
#define MB_ART_NVS      4  /**< ACPI NVS Memory (must not be used by the OS */
#define MB_ART_UNUSABLE 5  /**< memory in which errors have been detected */


/** Drive Info structure.  */
typedef struct
{
  l4_uint32_t size;		/** <The size of this structure.  */
  l4_uint8_t drive_number;	/** <The BIOS drive number.  */
  l4_uint8_t drive_mode;	/** <The access mode (see below).  */
  l4_uint16_t drive_cylinders;	/** <number of cylinders  */
  l4_uint8_t drive_heads;	/** <number of heads */
  l4_uint8_t drive_sectors;	/** <number of sectors per track */
  l4_uint16_t drive_ports[0];	/** <Array of I/O ports used for the drive. */
} l4util_mb_drive_t;

/* Drive Mode.  */
#define MB_DI_CHS_MODE		0
#define MB_DI_LBA_MODE		1


/** APM BIOS info.  */
typedef struct
{
  l4_uint16_t version;
  l4_uint16_t cseg;
  l4_uint32_t offset;
  l4_uint16_t cseg_16;
  l4_uint16_t dseg_16;
  l4_uint16_t cseg_len;
  l4_uint16_t cseg_16_len;
  l4_uint16_t dseg_16_len;
} l4util_mb_apm_t;


/** VBE controller information. */
typedef struct
{
  l4_uint8_t signature[4];
  l4_uint16_t version;
  l4_uint32_t oem_string;
  l4_uint32_t capabilities;
  l4_uint32_t video_mode;
  l4_uint16_t total_memory;
  l4_uint16_t oem_software_rev;
  l4_uint32_t oem_vendor_name;
  l4_uint32_t oem_product_name;
  l4_uint32_t oem_product_rev;
  l4_uint8_t reserved[222];
  l4_uint8_t oem_data[256];
} __attribute__((packed)) l4util_mb_vbe_ctrl_t;


/** VBE mode information. */
typedef struct
{
  /** @name all VESA versions
  * @{ */
  l4_uint16_t mode_attributes;
  l4_uint8_t win_a_attributes;
  l4_uint8_t win_b_attributes;
  l4_uint16_t win_granularity;
  l4_uint16_t win_size;
  l4_uint16_t win_a_segment;
  l4_uint16_t win_b_segment;
  l4_uint32_t win_func;
  l4_uint16_t bytes_per_scanline;
  /** @} */

  /** @name >= VESA version 1.2
   * @{ */
  l4_uint16_t x_resolution;
  l4_uint16_t y_resolution;
  l4_uint8_t x_char_size;
  l4_uint8_t y_char_size;
  l4_uint8_t number_of_planes;
  l4_uint8_t bits_per_pixel;
  l4_uint8_t number_of_banks;
  l4_uint8_t memory_model;
  l4_uint8_t bank_size;
  l4_uint8_t number_of_image_pages;
  l4_uint8_t reserved0;
  /** @} */

  /** @name direct color
   * @{ */
  l4_uint8_t red_mask_size;
  l4_uint8_t red_field_position;
  l4_uint8_t green_mask_size;
  l4_uint8_t green_field_position;
  l4_uint8_t blue_mask_size;
  l4_uint8_t blue_field_position;
  l4_uint8_t reserved_mask_size;
  l4_uint8_t reserved_field_position;
  l4_uint8_t direct_color_mode_info;
  /** @} */

  /** @name >= VESA version 2.0 
   * @{*/
  l4_uint32_t phys_base;
  l4_uint32_t reserved1;
  l4_uint16_t reversed2;
  /** @} */

  /** @name >= VESA version 3.0
   * @{*/
  l4_uint16_t linear_bytes_per_scanline;
  l4_uint8_t banked_number_of_image_pages;
  l4_uint8_t linear_number_of_image_pages;
  l4_uint8_t linear_red_mask_size;
  l4_uint8_t linear_red_field_position;
  l4_uint8_t linear_green_mask_size;
  l4_uint8_t linear_green_field_position;
  l4_uint8_t linear_blue_mask_size;
  l4_uint8_t linear_blue_field_position;
  l4_uint8_t linear_reserved_mask_size;
  l4_uint8_t linear_reserved_field_position;
  l4_uint32_t max_pixel_clock;

  /* The VBE spec says this structure should have a size of 256 bytes but
   * the described structure layout is only 255 bytes... */
  l4_uint8_t reserved3[189 + 1];
  /** @} */
} __attribute__ ((packed)) l4util_mb_vbe_mode_t;


/**
 * \anchor struct_l4util_mb_info
 *  MultiBoot Info description
 *
 *  This is the struct passed to the boot image.  This is done by placing
 *  its address in the EAX register.
 */

typedef struct
{
  l4_uint32_t flags;		/**< MultiBoot info version number */
  l4_uint32_t mem_lower;	/**< available memory below 1MB */
  l4_uint32_t mem_upper;	/**< available memory starting from 1MB [kB] */
  l4_uint32_t boot_device;	/**< "root" partition */
  l4_uint32_t cmdline;		/**< Kernel command line */
  l4_uint32_t mods_count;	/**< number of modules */
  l4_uint32_t mods_addr;	/**< module list */

  union
  {
    struct
    {
      /** (a.out) Kernel symbol table info */
      l4_uint32_t tabsize;
      l4_uint32_t strsize;
      l4_uint32_t addr;
      l4_uint32_t pad;
    }
    a;
    
    struct
    {
      /** (ELF) Kernel section header table */
      l4_uint32_t num;
      l4_uint32_t size;
      l4_uint32_t addr;
      l4_uint32_t shndx;
    }
    e;
  }
  syms;
  
  l4_uint32_t mmap_length;	/**< size of memory mapping buffer */
  l4_uint32_t mmap_addr;	/**< address of memory mapping buffer */
  l4_uint32_t drives_length;	/**< size of drive info buffer */
  l4_uint32_t drives_addr;	/**< address of driver info buffer */
  l4_uint32_t config_table;	/**< ROM configuration table */
  l4_uint32_t boot_loader_name;	/**< Boot Loader Name */
  l4_uint32_t apm_table;	/**< APM table */
  l4_uint32_t vbe_ctrl_info;	/**< VESA video contoller info */
  l4_uint32_t vbe_mode_info;	/**< VESA video mode info */
  l4_uint16_t vbe_mode;		/**< VESA video mode number */
  l4_uint16_t vbe_interface_seg; /**< VESA segment of prot BIOS interface */
  l4_uint16_t vbe_interface_off; /**< VESA offset of prot BIOS interface */
  l4_uint16_t vbe_interface_len; /**< VESA lenght of prot BIOS interface */
} l4util_mb_info_t;

#endif /* ! __ASSEMBLY__ */

/**
 *  Flags to be set in the 'flags' parameter above
 */

/** is there basic lower/upper memory information? */
#define L4UTIL_MB_MEMORY		0x00000001

/** is there a boot device set? */
#define L4UTIL_MB_BOOTDEV		0x00000002

/** is the command-line defined? */
#define L4UTIL_MB_CMDLINE		0x00000004

/** are there modules to do something with? */
#define L4UTIL_MB_MODS			0x00000008

/* These next two are mutually exclusive */
/** is there a symbol table loaded? */
#define L4UTIL_MB_AOUT_SYMS		0x00000010

/** is there an ELF section header table? */
#define L4UTIL_MB_ELF_SHDR		0x00000020

/** is there a full memory map? */
#define L4UTIL_MB_MEM_MAP		0x00000040

/** Is there drive info?  */
#define L4UTIL_MB_DRIVE_INFO		0x00000080

/** Is there a config table?  */
#define L4UTIL_MB_CONFIG_TABLE		0x00000100

/** Is there a boot loader name?  */
#define L4UTIL_MB_BOOT_LOADER_NAME	0x00000200

/** Is there a APM table?  */
#define L4UTIL_MB_APM_TABLE		0x00000400

/** Is there video information?  */
#define L4UTIL_MB_VIDEO_INFO		0x00000800


/** If we are multiboot-compliant, this value is present in the eax register */
#define L4UTIL_MB_VALID			0x2BADB002UL
#define L4UTIL_MB_VALID_ASM		0x2BADB002


#endif

