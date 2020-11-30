/**
 * \file
 * \brief   Region map interface, C interface.
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
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
 * \defgroup api_l4re_c_rm Region map interface
 * \ingroup api_l4re_c
 * \brief Region map C interface.
 */

#include <l4/re/env.h>
#include <l4/re/c/dataspace.h>

EXTERN_C_BEGIN

/**
 * \brief Flags for region operations.
 * \ingroup api_l4re_c_rm
 */
enum l4re_rm_flags_values {
  L4RE_RM_F_R    = L4RE_DS_F_R, /**< Region is read-only */
  L4RE_RM_F_W    = L4RE_DS_F_W,
  L4RE_RM_F_X    = L4RE_DS_F_X,
  L4RE_RM_F_RX   = L4RE_DS_F_RX,
  L4RE_RM_F_RW   = L4RE_DS_F_RW,
  L4RE_RM_F_RWX  = L4RE_DS_F_RWX,

  L4RE_RM_F_NO_ALIAS     = 0x200, /**< The region contains exclusive memory that is not mapped anywhere else */
  L4RE_RM_F_PAGER        = 0x400, /**< Region has a pager */
  L4RE_RM_F_RESERVED     = 0x800, /**< Region is reserved (blocked) */

  L4RE_RM_CACHING_SHIFT    = 4, /**< Start of region mapper cache bits */

  /** Mask of all region manager cache bits */
  L4RE_RM_F_CACHING      = L4RE_DS_F_CACHING_MASK,

  L4RE_RM_REGION_FLAGS = 0xffff, /**< Mask of all region flags */

  /** Cache bits for normal cacheable memory */
  L4RE_RM_F_CACHE_NORMAL   = L4RE_DS_F_NORMAL,

  /** Cache bits for buffered (write combining) memory */
  L4RE_RM_F_CACHE_BUFFERED = L4RE_DS_F_BUFFERABLE,

  /** Cache bits for uncached memory */
  L4RE_RM_F_CACHE_UNCACHED = L4RE_DS_F_UNCACHEABLE,

  L4RE_RM_F_SEARCH_ADDR  = 0x020000, /**< Search for a suitable address range */
  L4RE_RM_F_IN_AREA      = 0x040000, /**< Search only in area, or map into area */
  L4RE_RM_F_EAGER_MAP    = 0x080000, /**< Eagerly map the attached data space in. */
  L4RE_RM_F_NO_EAGER_MAP = 0x100000, /**< Prevent eager mapping of the attached data space. */
  L4RE_RM_F_ATTACH_FLAGS = 0x1f0000, /**< Mask of all attach flags */
};

typedef l4_uint32_t l4re_rm_flags_t;
typedef l4_uint64_t l4re_rm_offset_t;

/**
 * \ingroup api_l4re_c_rm
 * \copybrief L4Re::Rm::reserve_area
 * \copydetails L4Re::Rm::reserve_area
 * \see L4Re::Rm::reserve_area
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_reserve_area(l4_addr_t *start, unsigned long size,
                     l4re_rm_flags_t flags, unsigned char align) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \copybrief L4Re::Rm::free_area
 * \copydetails L4Re::Rm::free_area
 * \see L4Re::Rm::free_area
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_free_area(l4_addr_t addr) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \copybrief L4Re::Rm::attach
 * \copydetails L4Re::Rm::attach
 * \see L4Re::Rm::attach
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_attach(void **start, unsigned long size, l4re_rm_flags_t flags,
               l4re_ds_t mem,
               l4re_rm_offset_t offs,
               unsigned char align) L4_NOTHROW;


/**
 * \ingroup api_l4re_c_rm
 * \brief Detach and unmap a region from the address space in the current task.
 *
 * \param addr  Address of the region to detach.
 *
 * \retval #L4Re::Rm::Detach_result  On success.
 * \retval -L4_ENOENT                No region found.
 * \retval <0                        IPC errors
 *
 * Frees a region in the virtual address space given by addr. The corresponding
 * part of the address space is now available again.
 *
 * Also \see L4Re::Rm::detach
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_detach(void *addr) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \brief Detach and unmap a region and return affected dataspace in the
 *        current task.
 *
 * \param      addr  Address of the region to detach.
 * \param[out] ds    Returns dataspace that is affected.
 *
 * \retval #L4Re::Rm::Detach_result  On success.
 * \retval -L4_ENOENT                No region found.
 * \retval <0                        IPC errors
 *
 * Frees a region in the virtual address space given by addr. The corresponding
 * part of the address space is now available again.
 *
 * Also \see L4Re::Rm::detach
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_detach_ds(void *addr, l4re_ds_t *ds) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \brief Detach and unmap in specified task.
 * \param addr   Address of the region to detach.
 * \param task   Task to unmap pages from, specify L4_INVALID_CAP to not unmap
 * \return 0 on success, <0 on error
 *
 * Also \see L4Re::Rm::detach
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_detach_unmap(l4_addr_t addr, l4_cap_idx_t task) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \brief Detach and unmap in specified task.
 *
 * \param      addr  Address of the region to detach.
 * \param[out] ds    Returns dataspace that is affected.
 * \param      task  Task to unmap pages from, specify L4_INVALID_CAP to not unmap
 *
 * \return 0 on success, <0 on error
 *
 * Also \see L4Re::Rm::detach
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_detach_ds_unmap(void *addr, l4re_ds_t *ds,
                        l4_cap_idx_t task) L4_NOTHROW;


/**
 * \ingroup api_l4re_c_rm
 * \copybrief L4Re::Rm::find
 * \copydetails L4Re::Rm::find
 * \see L4Re::Rm::find
 */
L4_CV L4_INLINE int
l4re_rm_find(l4_addr_t *addr, unsigned long *size,
             l4re_rm_offset_t *offset,
             l4re_rm_flags_t *flags, l4re_ds_t *m) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \brief Dump region map internal data structures.
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE void
l4re_rm_show_lists(void) L4_NOTHROW;


/*
 * Variants of functions that also take a capability of the region map
 * service.
 */


/**
 * \ingroup api_l4re_c_rm
 * \see L4Re::Rm::reserve_area
 */
L4_CV int
l4re_rm_reserve_area_srv(l4_cap_idx_t rm, l4_addr_t *start, unsigned long size,
                         l4re_rm_flags_t flags, unsigned char align) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \see L4Re::Rm::free_area
 */
L4_CV int
l4re_rm_free_area_srv(l4_cap_idx_t rm, l4_addr_t addr) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \see L4Re::Rm::attach
 */
L4_CV int
l4re_rm_attach_srv(l4_cap_idx_t rm, void **start, unsigned long size,
                   l4re_rm_flags_t flags, l4re_ds_t mem,
                   l4re_rm_offset_t offs,
                   unsigned char align) L4_NOTHROW;


/**
 * \see L4Re::Rm::detach
 * \ingroup api_l4re_c_rm
 */
L4_CV int
l4re_rm_detach_srv(l4_cap_idx_t rm, l4_addr_t addr,
                   l4re_ds_t *ds, l4_cap_idx_t task) L4_NOTHROW;


/**
 * \see L4Re::Rm::find
 * \ingroup api_l4re_c_rm
 */
L4_CV int
l4re_rm_find_srv(l4_cap_idx_t rm, l4_addr_t *addr,
                 unsigned long *size, l4re_rm_offset_t *offset,
                 l4re_rm_flags_t *flags, l4re_ds_t *m) L4_NOTHROW;

/**
 * \brief Dump region map internal data structures.
 * \ingroup api_l4re_c_rm
 */
L4_CV void
l4re_rm_show_lists_srv(l4_cap_idx_t rm) L4_NOTHROW;


/********** Implementations ***************************/

L4_CV L4_INLINE int
l4re_rm_reserve_area(l4_addr_t *start, unsigned long size,
                     l4re_rm_flags_t flags, unsigned char align) L4_NOTHROW
{
  return l4re_rm_reserve_area_srv(l4re_global_env->rm, start, size,
                                  flags, align);
}

L4_CV L4_INLINE int
l4re_rm_free_area(l4_addr_t addr) L4_NOTHROW
{
  return l4re_rm_free_area_srv(l4re_global_env->rm, addr);
}

L4_CV L4_INLINE int
l4re_rm_attach(void **start, unsigned long size, l4re_rm_flags_t flags,
               l4re_ds_t mem, l4re_rm_offset_t offs,
               unsigned char align) L4_NOTHROW
{
  return l4re_rm_attach_srv(l4re_global_env->rm, start, size,
                            flags, mem, offs, align);
}


L4_CV L4_INLINE int
l4re_rm_detach(void *addr) L4_NOTHROW
{
  return l4re_rm_detach_srv(l4re_global_env->rm,
                            (l4_addr_t)addr, 0, L4_BASE_TASK_CAP);
}

L4_CV L4_INLINE int
l4re_rm_detach_unmap(l4_addr_t addr, l4_cap_idx_t task) L4_NOTHROW
{
  return l4re_rm_detach_srv(l4re_global_env->rm, addr, 0, task);
}

L4_CV L4_INLINE int
l4re_rm_detach_ds(void *addr, l4re_ds_t *ds) L4_NOTHROW
{
  return l4re_rm_detach_srv(l4re_global_env->rm, (l4_addr_t)addr,
                            ds, L4_BASE_TASK_CAP);
}

L4_CV L4_INLINE int
l4re_rm_detach_ds_unmap(void *addr, l4re_ds_t *ds, l4_cap_idx_t task) L4_NOTHROW
{
  return l4re_rm_detach_srv(l4re_global_env->rm, (l4_addr_t)addr,
                            ds, task);
}

L4_CV L4_INLINE int
l4re_rm_find(l4_addr_t *addr, unsigned long *size,
             l4re_rm_offset_t *offset,
             l4re_rm_flags_t *flags, l4re_ds_t *m) L4_NOTHROW
{
  return l4re_rm_find_srv(l4re_global_env->rm, addr, size, offset, flags, m);
}

L4_CV L4_INLINE void
l4re_rm_show_lists(void) L4_NOTHROW
{
  l4re_rm_show_lists_srv(l4re_global_env->rm);
}

EXTERN_C_END
