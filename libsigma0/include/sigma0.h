/**
 * \file
 * \brief Sigma0 interface
 * \ingroup l4sigma0_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __L4_SIGMA0_SIGMA0_H
#define __L4_SIGMA0_SIGMA0_H

/**
 * \defgroup l4sigma0_api Sigma0 API
 *
 * \brief Sigma0 API bindings.
 *
 * Convenience bindings for the Sigma0 protocol.
 */

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>
#include <l4/sys/kip.h>

/**
 * \defgroup l4sigma0_api_internal Internal constants
 * \ingroup l4sigma0_api
 * \brief    Internal sigma0 definitions.
 */
/*@{*/
#undef SIGMA0_REQ_MAGIC
#undef SIGMA0_REQ_MASK

# define SIGMA0_REQ_MAGIC		~0xFFUL    /**< Request magic */
# define SIGMA0_REQ_MASK		~0xFFUL    /**< Request mask */

/* Starting with 0x60 allows to detect components which still use the old
 * constants (0x00 ... 0x50) */
#define SIGMA0_REQ_ID_MASK		  0xF0     /**< ID mask */
#define SIGMA0_REQ_ID_FPAGE_RAM		  0x60     /**< RAM */
#define SIGMA0_REQ_ID_FPAGE_IOMEM	  0x70     /**< I/O memory */
#define SIGMA0_REQ_ID_FPAGE_IOMEM_CACHED  0x80     /**< Cached I/O memory */
#define SIGMA0_REQ_ID_FPAGE_ANY		  0x90     /**< Any */
#define SIGMA0_REQ_ID_KIP		  0xA0     /**< KIP */
#define SIGMA0_REQ_ID_TBUF		  0xB0     /**< TBUF */
#define SIGMA0_REQ_ID_DEBUG_DUMP	  0xC0     /**< Debug dump */
#define SIGMA0_REQ_ID_NEW_CLIENT	  0xD0     /**< New client */

#define SIGMA0_IS_MAGIC_REQ(d1)	\
  ((d1 & SIGMA0_REQ_MASK) == SIGMA0_REQ_MAGIC)     /**< Check if magic */

#define SIGMA0_REQ(x) \
  (SIGMA0_REQ_MAGIC + SIGMA0_REQ_ID_ ## x)         /**< Construct */

/* Use these constants in your code! */
#define SIGMA0_REQ_FPAGE_RAM            (SIGMA0_REQ(FPAGE_RAM))          /**< RAM */
#define SIGMA0_REQ_FPAGE_IOMEM	        (SIGMA0_REQ(FPAGE_IOMEM))        /**< I/O memory */
#define SIGMA0_REQ_FPAGE_IOMEM_CACHED   (SIGMA0_REQ(FPAGE_IOMEM_CACHED)) /**< Cache I/O memory*/
#define SIGMA0_REQ_FPAGE_ANY            (SIGMA0_REQ(FPAGE_ANY))          /**< Any */
#define SIGMA0_REQ_KIP                  (SIGMA0_REQ(KIP))                /**< KIP */
#define SIGMA0_REQ_TBUF	                (SIGMA0_REQ(TBUF))               /**< TBUF */
#define SIGMA0_REQ_DEBUG_DUMP           (SIGMA0_REQ(DEBUG_DUMP))         /**< Debug dump */
#define SIGMA0_REQ_NEW_CLIENT           (SIGMA0_REQ(NEW_CLIENT))         /**< New client */
/*@}*/

/**
 * \addtogroup l4sigma0_api
 */
/*@{*/

/**
 * \brief Return flags of libsigma0 functions.
 */
enum l4sigma0_return_flags_t {
  L4SIGMA0_OK,           /**< Ok */
  L4SIGMA0_NOTALIGNED,   /**< Phys, virt or size not aligned */
  L4SIGMA0_IPCERROR,     /**< IPC error */
  L4SIGMA0_NOFPAGE,      /**< No fpage received */
  L4SIGMA0_4,
  L4SIGMA0_5,
  L4SIGMA0_SMALLERFPAGE, /**< Superpage requested but smaller flexpage received */
};

EXTERN_C_BEGIN

/**
 * \brief  Map the kernel info page from pager to addr.
 *
 * \param  sigma0    Capability selector for the sigma0 gate.
 * \param  addr      Start of the receive window to receive KIP in.
 * \param  log2_size Size of the receive window to receive KIP in.
 * \return Address KIP was mapped to, 0 indicates an error.
 */
L4_CV l4_kernel_info_t *
l4sigma0_map_kip(l4_cap_idx_t sigma0, void *addr, unsigned log2_size);

/**
 * \brief Request a memory mapping from sigma0.
 * \param sigma0 ID of service talking the sigma0 protocol.
 * \param phys  the physical address of the requested page (must be at least
 *              aligned to the minimum page size).
 * \param virt  the virtual address where the paged should be mapped in the
 *              local address space (must be at least aligned to the minimum
 *              page size).
 * \param size  the size of the requested page, this must be a multiple of
 *              the minimum page size.
 *
 * \return 0 on success, !0 else (see l4sigma0_map_errstr()).
 */
L4_CV int l4sigma0_map_mem(l4_cap_idx_t sigma0,
                           l4_addr_t phys, l4_addr_t virt, l4_addr_t size);

/**
 * \brief Request IO memory from sigma0.
 *
 * \param sigma0  Capability to pager implementing the Sigma0 protocol.
 * \param phys    The physical address to be requested (page aligned).
 * \param virt    The virtual address where the memory should be mapped to
 *                (page aligned).
 * \param size    The size of the IO memory area to be mapped (multiple of
 *                page size)
 * \param cached  Requests cacheable IO memory if 1, and uncached if 0.
 *
 * \retval 0                     Success.
 * \retval -L4SIGMA0_NOTALIGNED  `phys`, `virt`, or `size` are not aligned.
 * \retval -L4SIGMA0_IPCERROR    IPC error.
 * \retval -L4SIGMA0_NOFPAGE     No fpage received.
 *
 * This function is similar to l4sigma0_map_mem(), the difference is that
 * it requests IO memory. IO memory is everything that is not known
 * to be normal RAM. Also ACPI tables or the BIOS memory is treated as IO
 * memory.
 *
 * See l4sigma0_map_errstr() to get a description of the return value.
 */
L4_CV int l4sigma0_map_iomem(l4_cap_idx_t sigma0, l4_addr_t phys,
                             l4_addr_t virt, l4_addr_t size, int cached);
/**
 * \brief Request an arbitrary free page of RAM.
 *
 * This function requests arbitrary free memory from sigma0. It should be used
 * whenever spare memory is needed, instead of requesting specific physical
 * memory with l4sigma0_map_mem().
 *
 * \param sigma0    usually the thread id of sigma0.
 * \param map_area the base address of the local virtual memory area where the
 *                 page should be mapped.
 * \param log2_map_size the size of the requested page log 2 (the size in
 *                      bytes is 2^log2_map_size). This must be at least the
 *                      minimal page size. By specifing larger sizes the
 *                      largest possible hardware page size will be used.
 * \retval base    physical address of the page received (i.e., the send base
 *                 of the received mapping if any).
 * \param  sz      Size to map by the server, in 2^sz bytes.
 *
 * \return 0 on success, !0 else (see l4sigma0_map_errstr()).
 */
L4_CV int l4sigma0_map_anypage(l4_cap_idx_t sigma0, l4_addr_t map_area,
                               unsigned log2_map_size, l4_addr_t *base,
                               unsigned sz);

/**
 * \brief Request Fiasco trace buffer.
 *
 * This is a Fiasco specific feature. Where you can request the kernel
 * internal trace buffer for user-level evaluation. This is for special
 * debugging tools, such as Ferret.
 *
 * \param sigma0 as usual the sigma0 thread id.
 * \param virt the virtual address where the trace buffer should be mapped,
 *
 * \return 0 on success, !0 else (see l4sigma0_map_errstr()).
 */
L4_CV int l4sigma0_map_tbuf(l4_cap_idx_t sigma0, l4_addr_t virt);

/**
 * \brief Request sigma0 to dump internal debug information.
 *
 * The debug information, such as internal memory maps, as well as
 * statistics about the internal allocators is dumped to the kernel debugger.
 *
 * \param sigma0 the sigma0 thread id.
 */
L4_CV void l4sigma0_debug_dump(l4_cap_idx_t sigma0);

/**
 * \brief Create a new IPC gate for a new Sigma0 client.
 * \param sigma0 Capability selector for sigma0 gate.
 * \param gate   Capability selector to use for the new gate.
 */
L4_CV int l4sigma0_new_client(l4_cap_idx_t sigma0, l4_cap_idx_t gate);

/**
 * \brief Get a user readable error messages for the return codes.
 *
 * \param err the error code reported by the *map* functions.
 * \return a string containing the error message.
 */
L4_INLINE char const *l4sigma0_map_errstr(int err);

/*@}*/


/* Implementations */

L4_INLINE char const *l4sigma0_map_errstr(int err)
{
  switch (err)
    {
    case  0: return "No error";
    case -1: return "Phys, virt or size not aligned";
    case -2: return "IPC error";
    case -3: return "No fpage received";
#ifndef SIGMA0_REQ_MAGIC
    case -4: return "Bad physical address (old protocol only)";
#endif
    case -6: return "Superpage requested but smaller flexpage received";
    case -7: return "Cannot map I/O memory cacheable (old protocol only)";
    default: return "Unknown error";
    }
}


EXTERN_C_END

#endif /* ! __L4_SIGMA0_SIGMA0_H */
