/**
 * \file
 * Memory description functions.
 * \ingroup l4_api
 */
/*
 * (c) 2007-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#ifndef __L4SYS__MEMDESC_H__
#define __L4SYS__MEMDESC_H__

#include <l4/sys/kip.h>

/**
 * \defgroup l4_kip_memdesc_api Memory descriptors (C version)
 * \ingroup l4_kip_api
 * C Interface for KIP memory descriptors.
 *
 * \includefile{l4/sys/memdesc.h}
 *
 * This module contains the C functions to access the memory descriptor in the
 * kernel interface page (KIP).
 */

/**
 * Type of a memory descriptor.
 * \ingroup l4_kip_memdesc_api
 */
enum l4_mem_type_t
{
  l4_mem_type_undefined    = 0x0, ///< Undefined, unused descriptor
  l4_mem_type_conventional = 0x1, ///< Conventional memory
  l4_mem_type_reserved     = 0x2, ///< Reserved memory for kernel etc.
  l4_mem_type_dedicated    = 0x3, ///< Dedicated memory (some device memory)
  l4_mem_type_shared       = 0x4, ///< Shared memory (not implemented)

  l4_mem_type_info         = 0xd, ///< Info from the boot loader
  l4_mem_type_bootloader   = 0xe, ///< Memory owned by the boot loader
  l4_mem_type_archspecific = 0xf, ///< Architecture specific memory (e.g., ACPI memory)
};

/**
 * Memory sub types for l4_mem_type_info descriptors
 * \ingroup l4_kip_memdesc_api
 */
enum l4_mem_info_sub_type_t
{
  l4_mem_info_acpi_rsdp = 0, /**< Physical address of the ACPI root pointer. */

  l4_mem_reserved_kernel = 0, /**< Kernel image. */
  l4_mem_reserved_heap   = 1, /**< Kernel heap. */
  l4_mem_reserved_mmio   = 2, /**< MMIO range reserved by kernel. */
};

/**
 * Memory sub types for l4_mem_type_archspecific descriptors
 * \ingroup l4_kip_memdesc_api
 */
enum l4_mem_archspecific_sub_type_common_t
{
  l4_mem_archspecific_acpi_tables = 3,  /**< Firmware ACPI tables. */
  l4_mem_archspecific_acpi_nvs    = 4,  /**< Firmware reserved address space. */
};


/**
 * Memory descriptor data structure.
 * \ingroup l4_kip_memdesc_api
 *
 * \note This data type is opaque, and must be accessed by the accessor
 * functions defined in this module.
 */
typedef struct l4_kernel_info_mem_desc_t
{
  /// \internal
  l4_umword_t l;
  /// \internal
  l4_umword_t h;
} l4_kernel_info_mem_desc_t;


/**
 * Get pointer to memory descriptors from KIP.
 * \ingroup l4_kip_memdesc_api
 */
L4_INLINE
l4_kernel_info_mem_desc_t *
l4_kernel_info_get_mem_descs(l4_kernel_info_t *kip) L4_NOTHROW;

/**
 * Get number of memory descriptors in KIP.
 * \ingroup l4_kip_memdesc_api
 *
 * \return Number of memory descriptors.
 */
L4_INLINE
unsigned
l4_kernel_info_get_num_mem_descs(l4_kernel_info_t *kip) L4_NOTHROW;

/**
 * Populate a memory descriptor.
 * \ingroup l4_kip_memdesc_api
 *
 * \param md        Pointer to memory descriptor
 * \param start     Start of region
 * \param end       End of region
 * \param type      Type of region
 * \param virt      1 if virtual region, 0 if physical region
 * \param sub_type  Sub type.
 */
L4_INLINE
void
l4_kernel_info_set_mem_desc(l4_kernel_info_mem_desc_t *md,
                            l4_addr_t start,
			    l4_addr_t end,
			    unsigned type,
			    unsigned virt,
			    unsigned sub_type) L4_NOTHROW;

/**
 * Get start address of the region described by the memory descriptor.
 * \ingroup l4_kip_memdesc_api
 *
 * \return Start address.
 */
L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_start(l4_kernel_info_mem_desc_t *md) L4_NOTHROW;

/**
 * Get end address of the region described by the memory descriptor.
 * \ingroup l4_kip_memdesc_api
 *
 * \return End address.
 */
L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_end(l4_kernel_info_mem_desc_t *md) L4_NOTHROW;

/**
 * Get type of the memory region.
 * \ingroup l4_kip_memdesc_api
 *
 * \return Type of the region (see #l4_mem_type_t).
 */
L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_type(l4_kernel_info_mem_desc_t *md) L4_NOTHROW;

/**
 * Get sub-type of memory region.
 * \ingroup l4_kip_memdesc_api
 *
 * \return Sub-type.
 *
 * The sub type is defined for architecture specific memory descriptors
 * (see #l4_mem_type_archspecific) and has architecture specific meaning.
 */
L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_subtype(l4_kernel_info_mem_desc_t *md) L4_NOTHROW;

/**
 * Get virtual flag of the memory descriptor.
 * \ingroup l4_kip_memdesc_api
 *
 * \return 1 if region is virtual memory, 0 if region is physical memory
 */
L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_is_virtual(l4_kernel_info_mem_desc_t *md) L4_NOTHROW;

/*************************************************************************
 * Implementations
 *************************************************************************/

L4_INLINE
l4_kernel_info_mem_desc_t *
l4_kernel_info_get_mem_descs(l4_kernel_info_t *kip) L4_NOTHROW
{
  return (l4_kernel_info_mem_desc_t *)((l4_addr_t)kip + kip->mem_descs);
}

L4_INLINE
unsigned
l4_kernel_info_get_num_mem_descs(l4_kernel_info_t *kip) L4_NOTHROW
{
  return kip->mem_descs_num;
}

L4_INLINE
void
l4_kernel_info_set_mem_desc(l4_kernel_info_mem_desc_t *md,
                            l4_addr_t start,
			    l4_addr_t end,
			    unsigned type,
			    unsigned virt,
			    unsigned sub_type) L4_NOTHROW
{
  md->l = (start & ~0x3ffUL) | (type & 0x0f) | ((sub_type << 4) & 0x0f0)
    | (virt ? 0x200 : 0x0);
  md->h = end;
}


L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_start(l4_kernel_info_mem_desc_t *md) L4_NOTHROW
{
  return md->l & ~0x3ffUL;
}

L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_end(l4_kernel_info_mem_desc_t *md) L4_NOTHROW
{
  return md->h | 0x3ffUL;
}

L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_type(l4_kernel_info_mem_desc_t *md) L4_NOTHROW
{
  return md->l & 0xf;
}

L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_subtype(l4_kernel_info_mem_desc_t *md) L4_NOTHROW
{
  return (md->l & 0xf0) >> 4;
}

L4_INLINE
l4_umword_t
l4_kernel_info_get_mem_desc_is_virtual(l4_kernel_info_mem_desc_t *md) L4_NOTHROW
{
  return md->l & 0x200;
}

#endif /* ! __L4SYS__MEMDESC_H__ */
