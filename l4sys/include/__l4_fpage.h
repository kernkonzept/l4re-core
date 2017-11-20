/**
 * \internal
 * \file
 * Common flex-page definitions.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

/**
 * \defgroup l4_fpage_api Flex pages
 * \ingroup l4_api
 * Flex-page related API.
 *
 * A flex page is a page with a variable size, that can describe memory,
 * IO-Ports (IA32 only), and sets of kernel objects.
 *
 * A flex page describes an always size aligned region of an address space.
 * The size is given in a log2 scale. This means the size in elements (bytes
 * for memory, ports for IO-Ports, and capabilities for kernel objects) is
 * always a power of two.
 *
 * A flex page also carries type and access right information for the
 * described region. The type information selects the address space in which
 * the flex page is valid. Access rights have a meaning depending on the
 * specific address space (type).
 *
 * There exists a special type for defining \em receive \em windows or for
 * the l4_task_unmap() method, that can be used to describe all address
 * spaces (all types) with a single flex page.
 */

/**
 * L4 flexpage structure
 * \ingroup l4_fpage_api
 */
enum l4_fpage_consts
{
  L4_FPAGE_RIGHTS_SHIFT = 0,  ///< Access permissions shift
  L4_FPAGE_TYPE_SHIFT   = 4,  ///< Flexpage type shift (memory, IO port, obj...)
  L4_FPAGE_SIZE_SHIFT   = 6,  ///< Flexpage size shift (log2-based)
  L4_FPAGE_ADDR_SHIFT   = 12, ///< Page address shift

  L4_FPAGE_RIGHTS_BITS = 4,   ///< Access permissions size
  L4_FPAGE_TYPE_BITS   = 2,   ///< Flexpage type size (memory, IO port, obj...)
  L4_FPAGE_SIZE_BITS   = 6,   ///< Flexpage size size (log2-based)
  L4_FPAGE_ADDR_BITS   = L4_MWORD_BITS - L4_FPAGE_ADDR_SHIFT,  ///< Page address size

  /// Mask to get the flexpage rights
  L4_FPAGE_RIGHTS_MASK  = ((1UL << L4_FPAGE_RIGHTS_BITS) - 1)
                          << L4_FPAGE_RIGHTS_SHIFT,
  L4_FPAGE_TYPE_MASK    = ((1UL << L4_FPAGE_TYPE_BITS)   - 1)
                          << L4_FPAGE_TYPE_SHIFT,
  L4_FPAGE_SIZE_MASK    = ((1UL << L4_FPAGE_SIZE_BITS)   - 1)
                          << L4_FPAGE_SIZE_SHIFT,
  L4_FPAGE_ADDR_MASK    = ~0UL << L4_FPAGE_ADDR_SHIFT,
};

/**
 * L4 flexpage type
 * \ingroup l4_fpage_api
 */
typedef union {
  l4_umword_t fpage;          ///< Raw value
  l4_umword_t raw;            ///< Raw value
} l4_fpage_t;

/** Constants for flexpages
 * \ingroup l4_fpage_api
 */
enum
{
  L4_WHOLE_ADDRESS_SPACE = 63 /**< Whole address space size */
};

/**
 * Send-flex-page types
 * \ingroup l4_fpage_api
 */
typedef struct {
  l4_umword_t snd_base;      ///< Offset in receive window (send base)
  l4_fpage_t fpage;          ///< Source flex-page descriptor
} l4_snd_fpage_t;


/** Flex-page rights
 * \ingroup l4_fpage_api
 */
enum L4_fpage_rights
{
  L4_FPAGE_X     = 1,                        /**< Executable flex page */
  L4_FPAGE_W     = 2,                        /**< Writable flex page */
  L4_FPAGE_RO    = 4,                        /**< Read-only flex page  */
  L4_FPAGE_RW    = L4_FPAGE_RO | L4_FPAGE_W, /**< Read-write flex page */
  L4_FPAGE_RX    = L4_FPAGE_RO | L4_FPAGE_X, /**< Read-execute flex page */
  L4_FPAGE_RWX   = L4_FPAGE_RW | L4_FPAGE_X, /**< Read-write-execute flex page */
};

/**
 * Cap-flex-page rights.
 * \ingroup l4_fpage_api
 *
 * Capabilities are modified or transfered with map and unmap operations. For
 * that capabilities are wrapped into flex-page objects. The flex-page carries
 * a set of rights the sender wants to hand over to the receiver along with the
 * capability.
 *
 * For the user only the 'S' and the 'W' right are visible. Other rights such as
 * the 'D' right are internal to the corresponding kernel object and cannot be
 * evaluated by the receiver.
 *
 * \note A thread can also map a capability from its task's capability
 * table with a reduced set of rights into another slot of its own capability
 * table.
 */
enum L4_cap_fpage_rights
{
  /**
   * Interface specific 'W' right for capability flex-pages.
   *
   * The semantics of the 'W' right is defined by the protocol. For example
   * in case of a dataspace cap, the 'W' right is needed to get a writable
   * dataspace.
   */
  L4_CAP_FPAGE_W     = 0x1,
  /**
   * Interface specific 'S' right for capability flex-pages.
   *
   * The semantics of the 'S' right is defined by the interface. The kernel
   * masks this right with the 'S' right of the IPC gate over which the
   * capability is mapped. That means that the receiver capability will only
   * have the 'S' right set if both the flex-page and the IPC gate have the 'S'
   * bit set.
   */
  L4_CAP_FPAGE_S     = 0x2,
  /**
   * Read right for capability flex-pages.
   *
   * This is always required, otherwise no capability is mapped.
   */
  L4_CAP_FPAGE_R     = 0x4,
  L4_CAP_FPAGE_RO    = 0x4, /**< \copydoc L4_CAP_FPAGE_R */
  /**
   * Delete right for capability flex-pages.
   *
   * This allows the receiver to delete the corresponding kernel object using
   * unmap() regardless of other tasks still holding a capability to the kernel
   * object. Such capabilities are set to an empty capability if the object is
   * deleted.
   */
  L4_CAP_FPAGE_D     = 0x8,
  /**
   * Read and interface specific 'W' right for capability flex-pages.
   *
   * The semantics of the 'W' right is defined by the interface.
   * \see L4_CAP_FPAGE_W
   */
  L4_CAP_FPAGE_RW    = L4_CAP_FPAGE_R | L4_CAP_FPAGE_W,
  /**
   * Read and interface specific 'S' right for capability flex-pages.
   *
   * The semantics of the 'S' right is defined by the interface.
   * \see L4_CAP_FPAGE_S
   */
  L4_CAP_FPAGE_RS    = L4_CAP_FPAGE_R | L4_CAP_FPAGE_S,
  /**
   * Read, interface specific 'W', and 'S' rights for capability flex-pages.
   *
   * The semantics of the 'W' and 'S' right are defined by the interface.
   * \see L4_CAP_FPAGE_R, L4_CAP_FPAGE_W, and L4_CAP_FPAGE_S
   */
  L4_CAP_FPAGE_RWS   = L4_CAP_FPAGE_RW | L4_CAP_FPAGE_S,
  /**
   * Full rights for capability flex-pages.
   *
   * \see L4_CAP_FPAGE_R, L4_CAP_FPAGE_W, L4_CAP_FPAGE_S, and L4_CAP_FPAGE_D
   */
  L4_CAP_FPAGE_RWSD  = L4_CAP_FPAGE_RWS | L4_CAP_FPAGE_D,
  /**
   * Read, write, and delete right for capability flex-pages.
   *
   * \see L4_CAP_FPAGE_R, L4_CAP_FPAGE_W, and L4_CAP_FPAGE_D
   */
  L4_CAP_FPAGE_RWD  = L4_CAP_FPAGE_RW | L4_CAP_FPAGE_D,
  /**
   * Read, 'S', and delete right for capability flex-pages.
   *
   * \see L4_CAP_FPAGE_R, L4_CAP_FPAGE_S, and L4_CAP_FPAGE_D
   */
  L4_CAP_FPAGE_RSD   = L4_CAP_FPAGE_RS | L4_CAP_FPAGE_D,
};

/** Flex-page type
 * \ingroup l4_fpage_api
 */
enum L4_fpage_type
{
  L4_FPAGE_SPECIAL = 0,
  L4_FPAGE_MEMORY  = 1,
  L4_FPAGE_IO      = 2,
  L4_FPAGE_OBJ     = 3,
};

/** Flex-page map control flags
 * \ingroup l4_fpage_api
 */
enum L4_fpage_control
{
  L4_FPAGE_CONTROL_OFFSET_SHIFT = 10,
  L4_FPAGE_CONTROL_MASK = ~0UL << L4_FPAGE_CONTROL_OFFSET_SHIFT,
};

/**
 * Flex-page map control for capabilities (snd_base)
 *
 * These rights need to be added to the snd_base when mapping and
 * control internal behavior. The exact meaning depends on the type
 * of capability (currently used only with IPC gates).
 *
 * \ingroup l4_fpage_api
 */
enum L4_obj_fpage_ctl
{
  L4_FPAGE_C_REF_CNT    = 0x00,  ///< Mapping is reference-counted (default).
  L4_FPAGE_C_NO_REF_CNT = 0x10,  ///< Don't increase the reference counter.

  L4_FPAGE_C_OBJ_RIGHT1 = 0x20,  ///< Object-type specific right.
  L4_FPAGE_C_OBJ_RIGHT2 = 0x40,  ///< Object-type specific right.
  L4_FPAGE_C_OBJ_RIGHT3 = 0x80,  ///< Object-type specific right.
  L4_FPAGE_C_OBJ_RIGHTS = 0xe0,  ///< All Object-type specific right bits.

  /**
   * The receiver may invoke IPC-gate-specific functions on the capability,
   * e.g. bind a thread to the gate and modify the label. Needed if the
   * receiver implements the server side of an IPC gate.
   */
  L4_FPAGE_C_IPCGATE_SVR = L4_FPAGE_C_OBJ_RIGHT1
};


/** Flex-page cacheability option
 * \ingroup l4_fpage_api
 */
enum l4_fpage_cacheability_opt_t
{
  /** Enable the cacheability option in a send flex page. */
  L4_FPAGE_CACHE_OPT   = 0x1,

  /** Cacheability option to enable caches for the mapping. */
  L4_FPAGE_CACHEABLE   = 0x3,

  /** Cacheability option to enable buffered writes for the mapping. */
  L4_FPAGE_BUFFERABLE  = 0x5,

  /** Cacheability option to disable caching for the mapping. */
  L4_FPAGE_UNCACHEABLE = 0x1
};


/** Special constants for IO flex pages
 * \ingroup l4_fpage_api
 */
enum
{
  /** Whole I/O address space size */
  L4_WHOLE_IOADDRESS_SPACE  = 16,

  /** Maximum I/O port address */
  L4_IOPORT_MAX             = (1L << L4_WHOLE_IOADDRESS_SPACE)
};



/**
 * Create a memory flex page.
 * \ingroup l4_fpage_api
 *
 * \param   address      Flex-page start address
 * \param   size         Flex-page size (log2), #L4_WHOLE_ADDRESS_SPACE to
 *                       specify the whole address space (with `address` 0)
 * \param   rights       Access rights, see #L4_fpage_rights
 *
 * \return  Memory flex page
 */
L4_INLINE l4_fpage_t
l4_fpage(l4_addr_t address, unsigned int size, unsigned char rights) L4_NOTHROW;

/**
 * Get a flex page, describing all address spaces at once.
 * \ingroup l4_fpage_api
 *
 * \return  Special \em all-spaces flex page.
 */
L4_INLINE l4_fpage_t
l4_fpage_all(void) L4_NOTHROW;

/**
 * Get an invalid flex page.
 * \ingroup l4_fpage_api
 *
 * \return  Special \em invalid flex page.
 */
L4_INLINE l4_fpage_t
l4_fpage_invalid(void) L4_NOTHROW;


/**
 * Create an IO-port flex page.
 * \ingroup l4_fpage_api
 *
 * \param   port         I/O-flex-page port base
 * \param   size         I/O-flex-page size (log2), #L4_WHOLE_IOADDRESS_SPACE to
 *                       specify the whole I/O address space (with `port` 0)
 *
 * \return  I/O flex page
 */
L4_INLINE l4_fpage_t
l4_iofpage(unsigned long port, unsigned int size) L4_NOTHROW;


/**
 * Create a kernel-object flex page.
 * \ingroup l4_fpage_api
 *
 * \param   obj       Base capability selector.
 * \param   order     Log2 size (number of capabilities).
 * \param   rights    Access rights, see #L4_cap_fpage_rights
 *
 * \return  Flex page for a set of kernel objects.
 *
 * \note #L4_CAP_FPAGE_R is always required, otherwise no capability is mapped.
 */
L4_INLINE l4_fpage_t
l4_obj_fpage(l4_cap_idx_t obj, unsigned int order, unsigned char rights) L4_NOTHROW;

/**
 * Test if the flex page is writable.
 * \ingroup l4_fpage_api
 *
 * \param   fp  Flex page.
 *
 * \retval !=0 if flex page is writable.
 * \retval ==0 if flex pags is not writable.
 */
L4_INLINE int
l4_is_fpage_writable(l4_fpage_t fp) L4_NOTHROW;


/**
 * \defgroup l4_msgitem_api Message Items
 * \ingroup l4_ipc_api
 * Message item related functions.
 *
 * Message items are typed items that can be transferred via IPC
 * operations. Message items are also used to specify receive windows for
 * typed items to be received.
 * Message items are placed in the message registers (MRs) of the UTCB of
 * the sending thread.
 * Receive items are placed in the buffer registers (BRs) of the UTCB
 * of the receiving thread.
 *
 * Message items are usually two-word data structures. The first 
 * word denotes the type of the message item (for example a memory flex-page,
 * io flex-page or object flex-page) and the second word contains
 * information depending on the type. There is actually one exception that is
 * a small (one word) receive buffer item for a single capability.
 */

/**
 * Create the first word for a map item for the memory space.
 * \ingroup l4_msgitem_api
 *
 * \param spot   Hot spot address, used to determine what is actually mapped
 *               when send and receive flex page have differing sizes.
 * \param cache  Cacheability hints for memory flex pages. See
 *               \link l4_fpage_api::l4_fpage_cacheability_opt_t
 *               Cacheability options \endlink
 * \param grant  Indicates if it is a map or a grant item.
 *
 * \return The value to be used as first word in a map item for memory.
 */
L4_INLINE l4_umword_t
l4_map_control(l4_umword_t spot, unsigned char cache, unsigned grant) L4_NOTHROW;

/**
 * Create the first word for a map item for the object space.
 * \ingroup l4_msgitem_api
 *
 * \param spot   Hot spot address, used to determine what is actually mapped
 *               when send and receive flex pages have different size.
 * \param grant  Indicates if it is a map item or a grant item.
 *
 * \return The value to be used as first word in a map item for kernel objects
 *         or IO-ports.
 */
L4_INLINE l4_umword_t
l4_map_obj_control(l4_umword_t spot, unsigned grant) L4_NOTHROW;

/**
 * Return rights from a flex page.
 * \ingroup l4_fpage_api
 *
 * \param f  Flex page
 *
 * \return Size part of the given flex page.
 */
L4_INLINE unsigned
l4_fpage_rights(l4_fpage_t f) L4_NOTHROW;

/**
 * Return type from a flex page.
 * \ingroup l4_fpage_api
 *
 * \param f  Flex page
 *
 * \return Type part of the given flex page.
 */
L4_INLINE unsigned
l4_fpage_type(l4_fpage_t f) L4_NOTHROW;

/**
 * Return size from a flex page.
 * \ingroup l4_fpage_api
 *
 * \param f  Flex page
 *
 * \return Size part of the given flex page.
 *
 * \see l4_fpage_memaddr(), l4_fpage_obj(), l4_fpage_ioport()
 */
L4_INLINE unsigned
l4_fpage_size(l4_fpage_t f) L4_NOTHROW;

/**
 * Return the page part from a flex page.
 * \ingroup l4_fpage_api
 *
 * \param f  Flex page
 *
 * \return Page part of the given flex page.
 *
 * \note The meaning of the page part depends on the flex-page type.
 */
L4_INLINE unsigned long
l4_fpage_page(l4_fpage_t f) L4_NOTHROW;

/**
 * Return the memory address from the memory flex page.
 * \ingroup l4_fpage_api
 *
 * \param f  Flex page
 *
 * \return Page address from the given memory flex page.
 *
 * \pre `f` must be a memory flex page (`l4_fpage_type(f) == L4_FPAGE_MEMORY`).
 *
 * The function does not enforce size alignment of the read memory address. The
 * caller must ensure the input fpage is correct.
 */
L4_INLINE l4_addr_t
l4_fpage_memaddr(l4_fpage_t f) L4_NOTHROW;

/**
 * Return the capability index from the object flex page.
 * \ingroup l4_fpage_api
 *
 * \param f  Flex page
 *
 * \return Capability index from the given object flex page.
 *
 * \pre `f` must be an object flex page (`l4_fpage_type(f) == L4_FPAGE_OBJ`)
 *
 * The function does not enforce size alignment of the read memory address. The
 * caller must ensure the input fpage is correct.
 */
L4_INLINE l4_cap_idx_t
l4_fpage_obj(l4_fpage_t f) L4_NOTHROW;

/**
 * Return the IO port number from the IO flex page.
 * \ingroup l4_fpage_api
 *
 * \param f  Flex page
 *
 * \return IO port number from the given IO flex page.
 *
 * \pre `f` must be an IO flex page (`l4_fpage_type(f) == L4_FPAGE_IO`) and
 *
 * The function does not enforce size alignment of the read memory address. The
 * caller must ensure the input fpage is correct.
 */
L4_INLINE unsigned long
l4_fpage_ioport(l4_fpage_t f) L4_NOTHROW;

/**
 * Set new right in a flex page.
 * \ingroup l4_fpage_api
 *
 * \param  src         Flex page
 * \param  new_rights  New rights
 *
 * \return Modified flex page with new rights.
 */
L4_INLINE l4_fpage_t
l4_fpage_set_rights(l4_fpage_t src, unsigned char new_rights) L4_NOTHROW;

/**
 * Test whether a given range is completely within an fpage.
 * \ingroup l4_fpage_api
 *
 * \param   fpage    Flex page
 * \param   addr     Address
 * \param   size     Size of range in log2.
 *
 * \retval ==0 The range is not completely in the fpage.
 * \retval !=0 The range is within the fpage.
 */
L4_INLINE int
l4_fpage_contains(l4_fpage_t fpage, l4_addr_t addr, unsigned size) L4_NOTHROW;

/**
 * Determine maximum flex page size of a region.
 * \ingroup l4_fpage_api
 *
 * \param order     Order value to start with (e.g. for memory
 *                  L4_LOG2_PAGESIZE would be used)
 * \param addr      Address to be covered by the flex page.
 * \param min_addr  Start of region / minimal address (including).
 * \param max_addr  End of region / maximal address (excluding).
 * \param hotspot   (Optional) hot spot.
 *
 * \return Maximum order (log2-size) possible.
 *
 * \note The start address of the flex-page can be determined with
 *       l4_trunc_size(addr, returnvalue)
 */
L4_INLINE unsigned char
l4_fpage_max_order(unsigned char order, l4_addr_t addr,
                   l4_addr_t min_addr, l4_addr_t max_addr,
                   l4_addr_t hotspot L4_DEFAULT_PARAM(0));

/*************************************************************************
 * Implementations
 *************************************************************************/

L4_INLINE unsigned
l4_fpage_rights(l4_fpage_t f) L4_NOTHROW
{
  return (f.raw & L4_FPAGE_RIGHTS_MASK) >> L4_FPAGE_RIGHTS_SHIFT;
}

L4_INLINE unsigned
l4_fpage_type(l4_fpage_t f) L4_NOTHROW
{
  return (f.raw & L4_FPAGE_TYPE_MASK) >> L4_FPAGE_TYPE_SHIFT;
}

L4_INLINE unsigned
l4_fpage_size(l4_fpage_t f) L4_NOTHROW
{
  return (f.raw & L4_FPAGE_SIZE_MASK) >> L4_FPAGE_SIZE_SHIFT;
}

L4_INLINE unsigned long
l4_fpage_page(l4_fpage_t f) L4_NOTHROW
{
  return (f.raw & L4_FPAGE_ADDR_MASK) >> L4_FPAGE_ADDR_SHIFT;
}

L4_INLINE unsigned long
l4_fpage_ioport(l4_fpage_t f) L4_NOTHROW
{
  return (f.raw & L4_FPAGE_ADDR_MASK) >> L4_FPAGE_ADDR_SHIFT;
}

L4_INLINE l4_addr_t
l4_fpage_memaddr(l4_fpage_t f) L4_NOTHROW
{
  return f.raw & L4_FPAGE_ADDR_MASK;
}

L4_INLINE l4_cap_idx_t
l4_fpage_obj(l4_fpage_t f) L4_NOTHROW
{
  return f.raw & L4_FPAGE_ADDR_MASK;
}

/** \internal */
L4_INLINE l4_fpage_t
__l4_fpage_generic(unsigned long address, unsigned int type,
                   unsigned int size, unsigned char rights) L4_NOTHROW;

L4_INLINE l4_fpage_t
__l4_fpage_generic(unsigned long address, unsigned int type,
                   unsigned int size, unsigned char rights) L4_NOTHROW
{
  l4_fpage_t t;
  t.raw =   ((rights  << L4_FPAGE_RIGHTS_SHIFT) & L4_FPAGE_RIGHTS_MASK)
          | ((type    << L4_FPAGE_TYPE_SHIFT)   & L4_FPAGE_TYPE_MASK)
	  | ((size    << L4_FPAGE_SIZE_SHIFT)   & L4_FPAGE_SIZE_MASK)
	  | ((address                       )   & L4_FPAGE_ADDR_MASK);
  return t;
}

L4_INLINE l4_fpage_t
l4_fpage_set_rights(l4_fpage_t src, unsigned char new_rights) L4_NOTHROW
{
  l4_fpage_t f;
  f.raw = ((L4_FPAGE_TYPE_MASK | L4_FPAGE_SIZE_MASK | L4_FPAGE_ADDR_MASK) & src.raw)
          | ((new_rights << L4_FPAGE_RIGHTS_SHIFT) & L4_FPAGE_RIGHTS_MASK);
  return f;
}

L4_INLINE l4_fpage_t
l4_fpage(l4_addr_t address, unsigned int size, unsigned char rights) L4_NOTHROW
{
  return __l4_fpage_generic(address, L4_FPAGE_MEMORY, size, rights);
}

L4_INLINE l4_fpage_t
l4_iofpage(unsigned long port, unsigned int size) L4_NOTHROW
{
  return __l4_fpage_generic(port << L4_FPAGE_ADDR_SHIFT, L4_FPAGE_IO, size, L4_FPAGE_RW);
}

L4_INLINE l4_fpage_t
l4_obj_fpage(l4_cap_idx_t obj, unsigned int order, unsigned char rights) L4_NOTHROW
{
  static_assert((unsigned long)L4_CAP_SHIFT >= L4_FPAGE_ADDR_SHIFT,
                "Capability index does not fit into fpage.");
  return __l4_fpage_generic(obj, L4_FPAGE_OBJ, order, rights);
}

L4_INLINE l4_fpage_t
l4_fpage_all(void) L4_NOTHROW
{
  return __l4_fpage_generic(0, 0, L4_WHOLE_ADDRESS_SPACE, 0);
}

L4_INLINE l4_fpage_t
l4_fpage_invalid(void) L4_NOTHROW
{
  return __l4_fpage_generic(0, 0, 0, 0);
}


L4_INLINE int
l4_is_fpage_writable(l4_fpage_t fp) L4_NOTHROW
{
  return l4_fpage_rights(fp) & L4_FPAGE_W;
}

L4_INLINE l4_umword_t
l4_map_control(l4_umword_t snd_base, unsigned char cache, unsigned grant) L4_NOTHROW
{
  return (snd_base & L4_FPAGE_CONTROL_MASK)
         | ((l4_umword_t)cache << 4) | L4_ITEM_MAP | grant;
}

L4_INLINE l4_umword_t
l4_map_obj_control(l4_umword_t snd_base, unsigned grant) L4_NOTHROW
{
  return l4_map_control(snd_base, 0, grant);
}

L4_INLINE int
l4_fpage_contains(l4_fpage_t fpage, l4_addr_t addr, unsigned log2size) L4_NOTHROW
{
  l4_addr_t fa = l4_fpage_memaddr(fpage);
  return (fa <= addr)
         && (fa + (1UL << l4_fpage_size(fpage)) >= addr + (1UL << log2size));
}

L4_INLINE unsigned char
l4_fpage_max_order(unsigned char order, l4_addr_t addr,
                   l4_addr_t min_addr, l4_addr_t max_addr,
                   l4_addr_t hotspot)
{
  while (order < 30 /* limit to 1GB flexpages */)
    {
      l4_addr_t mask;
      l4_addr_t base = l4_trunc_size(addr, order + 1);
      if (base < min_addr)
        return order;

      if (base + (1UL << (order + 1)) - 1 > max_addr - 1)
        return order;

      mask = ~(~0UL << (order + 1));
      if (hotspot == ~0UL || ((addr ^ hotspot) & mask))
        break;

      ++order;
    }

  return order;
}
