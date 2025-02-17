/**
 * \internal
 * \file
 * Common flexpage definitions.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/compiler.h>

/**
 * \defgroup l4_fpage_api Flexpages
 * \ingroup l4_api
 * Flexpage-related API.
 *
 * A flexpage is a page with a variable size, that can describe memory,
 * IO-Ports (IA32 only), and sets of kernel objects.
 *
 * A flexpage describes an always size aligned region of an address space.
 * The size is given in a log2 scale. This means the size in elements (bytes
 * for memory, ports for IO-Ports, and capabilities for kernel objects) is
 * always a power of two.
 *
 * A flexpage also carries type and access right information for the
 * described region. The type information selects the address space in which
 * the flexpage is valid. Access rights have a meaning depending on the
 * specific address space (type).
 *
 * There exists a special type for defining \em receive \em windows or for
 * the l4_task_unmap() method, that can be used to describe all address
 * spaces (all types) with a single flexpage.
 *
 * \includefile{l4/sys/types.h}
 */

/**
 * L4 flexpage structure
 * \ingroup l4_fpage_api
 */
enum L4_fpage_consts
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
  /// Specify as flexpage rights during grant.
  L4_FPAGE_RIGHTS_ALL   = L4_FPAGE_RIGHTS_MASK,
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
  /**
   * Whole address space size. This value does not only specify the log2 size
   * of the biggest possible memory flexpage. It can be also used as size for
   * a special flexpage to define a flexpage which completely covers all
   * spaces.
   */
  L4_WHOLE_ADDRESS_SPACE = 63
};

/**
 * Send-flexpage types
 * \ingroup l4_msgitem_api
 */
typedef struct {
  l4_umword_t snd_base;      ///< Offset in receive window (send base)
  l4_fpage_t fpage;          ///< Source flexpage descriptor
} l4_snd_fpage_t;


/**
 * Memory and IO port flexpage rights
 * \ingroup l4_fpage_api
 *
 * For IO flexpages, bit 1 and bit 2 are a combined read/write right. In a map
 * operation, the receiver receives the IO port capability when the sender
 * possesses it and at least one of these bits is present. For an unmap
 * operation, the absence of one of those bits is sufficient to unmap the IO
 * port capability.
 *
 * Note that more memory attributes can be specified in a send item, see
 * #l4_fpage_cacheability_opt_t.
 */
enum L4_fpage_rights
{
  L4_FPAGE_X     = 1,                        /**< Executable flexpage */
  L4_FPAGE_W     = 2,                        /**< Writable flexpage */
  L4_FPAGE_RO    = 4,                        /**< Read-only flexpage  */
  L4_FPAGE_RW    = L4_FPAGE_RO | L4_FPAGE_W, /**< Read-write flexpage */
  L4_FPAGE_RX    = L4_FPAGE_RO | L4_FPAGE_X, /**< Read-execute flexpage */
  L4_FPAGE_RWX   = L4_FPAGE_RW | L4_FPAGE_X, /**< Read-write-execute flexpage */
};

/**
 * Object flexpage rights.
 * \ingroup l4_fpage_api
 *
 * Capabilities are modified or transferred with map and unmap operations. For
 * that, capabilities are wrapped into flexpage objects. The flexpage carries
 * a set of rights the sender wants to hand over to the receiver along with the
 * capability.
 *
 * For the user only the 'S' and the 'W' right are visible. Other rights such as
 * the 'D' right are internal to the corresponding kernel object and cannot be
 * evaluated by the receiver.
 *
 * Note that additional object attributes and permissions can be specified in a
 * send item, see #L4_obj_fpage_ctl.
 *
 * \note A thread can also map a capability from its task's capability
 * table with a reduced set of rights into another slot of its own capability
 * table.
 */
enum L4_cap_fpage_rights
{
  /**
   * Interface specific 'W' right for capability flexpages.
   *
   * The semantics of the 'W' right is defined by the protocol. For example
   * in case of a dataspace cap, the 'W' right is needed to get a writable
   * dataspace.
   */
  L4_CAP_FPAGE_W     = 0x1,
  /**
   * Interface specific 'S' right for capability flexpages.
   *
   * The semantics of the 'S' right is defined by the interface. When
   * transferring object capabilities via IPC, the kernel masks this right with
   * the 'S' right of the capability used to address the IPC partner. Thus, the
   * 'S' right of sent capabilities is only transferred if both the flexpage
   * and the IPC gate or thread capability specifying the IPC partner have the
   * 'S' right. For L4::Task::map(), the 'S' right is only transferred if the
   * flexpage, the source and destination task capabilities have the 'S' right.
   */
  L4_CAP_FPAGE_S     = 0x2,
  /**
   * Read right for capability flexpages.
   *
   * This is always required, otherwise no capability is mapped.
   */
  L4_CAP_FPAGE_R     = 0x4,
  L4_CAP_FPAGE_RO    = 0x4, /**< \copydoc L4_CAP_FPAGE_R */
  /**
   * Delete right for capability flexpages.
   *
   * This allows the receiver to delete the corresponding kernel object using
   * unmap() regardless of other tasks still holding a capability to the kernel
   * object. Such capabilities are set to an empty capability if the object is
   * deleted.
   */
  L4_CAP_FPAGE_D     = 0x8,
  /**
   * Read and interface specific 'W' right for capability flexpages.
   *
   * The semantics of the 'W' right is defined by the interface.
   * \see L4_CAP_FPAGE_W
   */
  L4_CAP_FPAGE_RW    = L4_CAP_FPAGE_R | L4_CAP_FPAGE_W,
  /**
   * Read and interface specific 'S' right for capability flexpages.
   *
   * The semantics of the 'S' right is defined by the interface.
   * \see L4_CAP_FPAGE_S
   */
  L4_CAP_FPAGE_RS    = L4_CAP_FPAGE_R | L4_CAP_FPAGE_S,
  /**
   * Read, interface specific 'W', and 'S' rights for capability flexpages.
   *
   * The semantics of the 'W' and 'S' right are defined by the interface.
   * \see L4_CAP_FPAGE_R, L4_CAP_FPAGE_W, and L4_CAP_FPAGE_S
   */
  L4_CAP_FPAGE_RWS   = L4_CAP_FPAGE_RW | L4_CAP_FPAGE_S,
  /**
   * Full rights for capability flexpages.
   *
   * \see L4_CAP_FPAGE_R, L4_CAP_FPAGE_W, L4_CAP_FPAGE_S, and L4_CAP_FPAGE_D
   */
  L4_CAP_FPAGE_RWSD  = L4_CAP_FPAGE_RWS | L4_CAP_FPAGE_D,
  /**
   * Read, write, and delete right for capability flexpages.
   *
   * \see L4_CAP_FPAGE_R, L4_CAP_FPAGE_W, and L4_CAP_FPAGE_D
   */
  L4_CAP_FPAGE_RWD  = L4_CAP_FPAGE_RW | L4_CAP_FPAGE_D,
  /**
   * Read, 'S', and delete right for capability flexpages.
   *
   * \see L4_CAP_FPAGE_R, L4_CAP_FPAGE_S, and L4_CAP_FPAGE_D
   */
  L4_CAP_FPAGE_RSD   = L4_CAP_FPAGE_RS | L4_CAP_FPAGE_D,
};

/** Flexpage type
 * \ingroup l4_fpage_api
 */
enum L4_fpage_type
{
  L4_FPAGE_SPECIAL = 0, ///< Special flexpage, either #l4_fpage_invalid() or
                        ///  #l4_fpage_all(); only supported by selected
                        ///  interfaces.
  L4_FPAGE_MEMORY  = 1, ///< Flexpage for memory spaces.
  L4_FPAGE_IO      = 2, ///< Flexpage for I/O port spaces.
  L4_FPAGE_OBJ     = 3, ///< Flexpage for object spaces.
};

/** Flexpage map control flags
 * \ingroup l4_fpage_api
 */
enum L4_fpage_control
{
  /// Number of bits an index must be shifted or an address must be aligned to
  /// in the control word.
  L4_FPAGE_CONTROL_OFFSET_SHIFT = 12,
  /// Mask for truncating the lower bits of the send base or the index of the
  /// control word.
  L4_FPAGE_CONTROL_MASK = ~0UL << L4_FPAGE_CONTROL_OFFSET_SHIFT,
};

/**
 * Attributes and additional permissions for object send items.
 *
 * These rights need to be added to the snd_base when mapping and
 * control internal behavior. The exact meaning depends on the type
 * of capability (currently used only with IPC gates).
 *
 * \ingroup l4_msgitem_api
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


/** Cacheability options for memory send items.
 * \ingroup l4_msgitem_api
 */
enum l4_fpage_cacheability_opt_t
{
  /// Enable the cacheability option in a memory send item.
  /// Without this flag, the options are copied from the sender.
  L4_FPAGE_CACHE_OPT   = 0x1,

  /// Cacheability option to enable caches for the mapping.
  /// Implies #L4_FPAGE_CACHE_OPT.
  L4_FPAGE_CACHEABLE   = 0x3,

  /// Cacheability option to enable buffered writes for the mapping.
  /// Implies #L4_FPAGE_CACHE_OPT.
  L4_FPAGE_BUFFERABLE  = 0x5,

  /// Cacheability option to disable caching for the mapping.
  /// Implies #L4_FPAGE_CACHE_OPT.
  L4_FPAGE_UNCACHEABLE = 0x1
};


/** Special constants for IO flexpages
 * \ingroup l4_fpage_api
 */
enum
{
  /**
   * Whole I/O address space size. In contrast to #L4_WHOLE_ADDRESS_SPACE,
   * this value forms the log2 size of the biggest possible I/O flexpage. */
  L4_WHOLE_IOADDRESS_SPACE  = 16,

  /** Maximum I/O port address plus 1. */
  L4_IOPORT_MAX             = (1L << L4_WHOLE_IOADDRESS_SPACE)
};



/**
 * Create a memory flexpage.
 * \ingroup l4_fpage_api
 *
 * \param   address      Flexpage start address
 * \param   order        Flexpage size (log2), #L4_WHOLE_ADDRESS_SPACE to
 *                       specify the whole address space (with `address` 0).
 *                       The minimum log2 size of a memory flexpage is defined
 *                       by #L4_LOG2_PAGESIZE according to the size of the
 *                       smallest virtual page supported by the MMU.
 * \param   rights       Access rights, see #L4_fpage_rights
 *
 * \return  Memory flexpage
 */
L4_INLINE l4_fpage_t
l4_fpage(l4_addr_t address, unsigned int order, unsigned char rights) L4_NOTHROW;

/**
 * Get a flexpage, describing all address spaces at once.
 * \ingroup l4_fpage_api
 *
 * \return  Special \em all-spaces flexpage.
 *
 * \note This flexpage can be used to define a receive window where the sender
 *       can send objects of any type, or for an unmap item completely covering
 *       all spaces of the target task. It does not make sense to use this
 *       flexpage as send item.
 */
L4_INLINE l4_fpage_t
l4_fpage_all(void) L4_NOTHROW;

/**
 * Get an invalid flexpage.
 * \ingroup l4_fpage_api
 *
 * \return  Special \em invalid flexpage.
 */
L4_INLINE l4_fpage_t
l4_fpage_invalid(void) L4_NOTHROW;


/**
 * Create an IO-port flexpage.
 * \ingroup l4_fpage_api
 *
 * \param   port         I/O-flexpage port base
 * \param   order        I/O-flexpage size (log2), #L4_WHOLE_IOADDRESS_SPACE to
 *                       specify the whole I/O address space (with `port` 0)
 *
 * \return  I/O flexpage
 */
L4_INLINE l4_fpage_t
l4_iofpage(unsigned long port, unsigned int order) L4_NOTHROW;


/**
 * Create a kernel-object flexpage.
 * \ingroup l4_fpage_api
 *
 * \param   obj       Base capability selector.
 * \param   order     Log2 size (number of capabilities).
 * \param   rights    Access rights, see #L4_cap_fpage_rights
 *
 * \return  Flexpage for a set of kernel objects.
 *
 * \note #L4_CAP_FPAGE_R is always required, otherwise no capability is mapped.
 */
L4_INLINE l4_fpage_t
l4_obj_fpage(l4_cap_idx_t obj, unsigned int order, unsigned char rights) L4_NOTHROW;

/**
 * Test if the flexpage is writable.
 * \ingroup l4_fpage_api
 *
 * \param   fp  Flexpage.
 *
 * \retval !=0 if flexpage is writable.
 * \retval ==0 if flexpage is not writable.
 */
L4_INLINE int
l4_is_fpage_writable(l4_fpage_t fp) L4_NOTHROW;


/**
 * \defgroup l4_msgitem_api Message Items
 * \ingroup l4_ipc_api
 * Message-item-related functionality.
 *
 * Message items are typed items that are used for transferring capabilities
 * during IPC. There are three sub-types of typed message items with variations
 * in the layout:
 *
 * 1. Typed message items set by the sender in its message registers (MRs) of
 *    the UTCB for specifying what shall be sent.
 * 2. Typed message items set by the receiver in its buffer registers (BRs) of
 *    the UTCB for specifying which types of capabilities may be received at
 *    which addresses.
 * 3. Typed message items set by the kernel in the receiver’s message registers
 *    (MRs) of the UTCB for providing information about the transfer to the
 *    receiver.
 *
 * They are abbreviated by *send item*, *receive item*, and *return item*,
 * respectively.
 *
 * A typed message item in the message registers (case 1 and case 3) always
 * consists of two words (even if it is a void item). The size of a typed
 * message item in the buffer registers (case 2) is determined by its first
 * word. The size is up to three words (see #L4_RCV_ITEM_SINGLE_CAP and
 * #L4_RCV_ITEM_FORWARD_MAPPINGS). A void item in the buffer registers consists
 * of a single word.
 *
 * \includefile{l4/sys/types.h}
 */

/**
 * Create the first word for a map item that is a send item for the memory
 * space.
 * \ingroup l4_msgitem_api
 *
 * \param spot   Hot spot address, used to determine what is actually mapped
 *               when send and receive flexpage have differing sizes.
 * \param cache  Cacheability hints for memory flexpages. See
 *               \link l4_fpage_cacheability_opt_t
 *               Cacheability options\endlink.
 * \param grant  Indicates if it is a map or a grant item. Allowed values:
 *               #L4_MAP_ITEM_MAP, #L4_MAP_ITEM_GRANT.
 *
 * \return The value to be used as first word in a send item for memory.
 */
L4_INLINE l4_umword_t
l4_map_control(l4_umword_t spot, unsigned char cache, unsigned grant) L4_NOTHROW;

/**
 * Create the first word for a map item that is a send item for the object
 * space.
 * \ingroup l4_msgitem_api
 *
 * \param spot   Hot spot address, used to determine what is actually mapped
 *               when send and receive flexpages have different size.
 * \param grant  Indicates if it is a map item or a grant item. Allowed values:
 *               #L4_MAP_ITEM_MAP, #L4_MAP_ITEM_GRANT.
 *
 * \return The value to be used as first word in a send item for kernel objects
 *         or IO-ports.
 */
L4_INLINE l4_umword_t
l4_map_obj_control(l4_umword_t spot, unsigned grant) L4_NOTHROW;

/**
 * Return rights from a flexpage.
 * \ingroup l4_fpage_api
 *
 * \param f  Flexpage
 *
 * \return Size part of the given flexpage.
 */
L4_INLINE unsigned
l4_fpage_rights(l4_fpage_t f) L4_NOTHROW;

/**
 * Return type from a flexpage.
 * \ingroup l4_fpage_api
 *
 * \param f  Flexpage
 *
 * \return Type part of the given flexpage.
 */
L4_INLINE unsigned
l4_fpage_type(l4_fpage_t f) L4_NOTHROW;

/**
 * Return size (log2) from a flexpage.
 * \ingroup l4_fpage_api
 *
 * \param f  Flexpage
 *
 * \return Size part of the given flexpage.
 *
 * \see l4_fpage_memaddr(), l4_fpage_obj(), l4_fpage_ioport()
 */
L4_INLINE unsigned
l4_fpage_size(l4_fpage_t f) L4_NOTHROW;

/**
 * Return the page part from a flexpage.
 * \ingroup l4_fpage_api
 *
 * \param f  Flexpage
 *
 * \return Page part of the given flexpage.
 *
 * \note The meaning of the page part depends on the flexpage type.
 */
L4_INLINE unsigned long
l4_fpage_page(l4_fpage_t f) L4_NOTHROW;

/**
 * Return the memory address from the memory flexpage.
 * \ingroup l4_fpage_api
 *
 * \param f  Flexpage
 *
 * \return Page address from the given memory flexpage.
 *
 * \pre `f` must be a memory flexpage (`l4_fpage_type(f) == L4_FPAGE_MEMORY`).
 *
 * The function does not enforce size alignment of the read memory address. The
 * caller must ensure the input fpage is correct.
 */
L4_INLINE l4_addr_t
l4_fpage_memaddr(l4_fpage_t f) L4_NOTHROW;

/**
 * Return the capability index from the object flexpage.
 * \ingroup l4_fpage_api
 *
 * \param f  Flexpage
 *
 * \return Capability index from the given object flexpage.
 *
 * \pre `f` must be an object flexpage (`l4_fpage_type(f) == L4_FPAGE_OBJ`)
 *
 * The function does not enforce size alignment of the read memory address. The
 * caller must ensure the input fpage is correct.
 */
L4_INLINE l4_cap_idx_t
l4_fpage_obj(l4_fpage_t f) L4_NOTHROW;

/**
 * Return the IO port number from the IO flexpage.
 * \ingroup l4_fpage_api
 *
 * \param f  Flexpage
 *
 * \return IO port number from the given IO flexpage.
 *
 * \pre `f` must be an IO flexpage (`l4_fpage_type(f) == L4_FPAGE_IO`) and
 *
 * The function does not enforce size alignment of the read memory address. The
 * caller must ensure the input fpage is correct.
 */
L4_INLINE unsigned long
l4_fpage_ioport(l4_fpage_t f) L4_NOTHROW;

/**
 * Set new right in a flexpage.
 * \ingroup l4_fpage_api
 *
 * \param  src         Flexpage
 * \param  new_rights  New rights
 *
 * \return Modified flexpage with new rights.
 */
L4_INLINE l4_fpage_t
l4_fpage_set_rights(l4_fpage_t src, unsigned char new_rights) L4_NOTHROW;

/**
 * Test whether a given range is completely within an fpage.
 * \ingroup l4_fpage_api
 *
 * \param   fpage    Flexpage
 * \param   addr     Address
 * \param   order    Size of range in log2.
 *
 * \retval ==0 The range is not completely in the fpage.
 * \retval !=0 The range is within the fpage.
 */
L4_INLINE int
l4_fpage_contains(l4_fpage_t fpage, l4_addr_t addr, unsigned order) L4_NOTHROW;

/**
 * Determine maximum flexpage size of a region.
 * \ingroup l4_fpage_api
 *
 * \param order     Order value to start with (e.g. for memory
 *                  L4_LOG2_PAGESIZE would be used)
 * \param addr      Address to be covered by the flexpage.
 * \param min_addr  Start of region / minimal address (including).
 * \param max_addr  End of region / maximal address (excluding).
 * \param hotspot   (Optional) hot spot.
 *
 * \return Maximum order (log2-size) possible.
 *
 * \note The start address of the flexpage can be determined with
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
                   unsigned int order, unsigned char rights) L4_NOTHROW;

L4_INLINE l4_fpage_t
__l4_fpage_generic(unsigned long address, unsigned int type,
                   unsigned int order, unsigned char rights) L4_NOTHROW
{
  l4_fpage_t t;
  t.raw =   ((rights  << L4_FPAGE_RIGHTS_SHIFT) & L4_FPAGE_RIGHTS_MASK)
          | ((type    << L4_FPAGE_TYPE_SHIFT)   & L4_FPAGE_TYPE_MASK)
	  | ((order   << L4_FPAGE_SIZE_SHIFT)   & L4_FPAGE_SIZE_MASK)
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
l4_fpage(l4_addr_t address, unsigned int order, unsigned char rights) L4_NOTHROW
{
  return __l4_fpage_generic(address, L4_FPAGE_MEMORY, order, rights);
}

L4_INLINE l4_fpage_t
l4_iofpage(unsigned long port, unsigned int order) L4_NOTHROW
{
  return __l4_fpage_generic(port << L4_FPAGE_ADDR_SHIFT, L4_FPAGE_IO, order, L4_FPAGE_RW);
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
  return __l4_fpage_generic(0, L4_FPAGE_SPECIAL, L4_WHOLE_ADDRESS_SPACE, 0);
}

L4_INLINE l4_fpage_t
l4_fpage_invalid(void) L4_NOTHROW
{
  return __l4_fpage_generic(0, L4_FPAGE_SPECIAL, 0, 0);
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
