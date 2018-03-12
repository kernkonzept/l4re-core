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
enum l4re_rm_flags_t {
  L4RE_RM_READ_ONLY    = 0x01, /**< \brief Region is read-only */
  L4RE_RM_NO_ALIAS     = 0x02, /**< \brief The region contains exclusive memory that is not mapped anywhere else */
  L4RE_RM_PAGER        = 0x04, /**< \brief Region has a pager */
  L4RE_RM_RESERVED     = 0x08, /**< \brief Region is reserved (blocked) */
  L4RE_RM_REGION_FLAGS = 0x0f, /**< \brief Mask of all region flags */

  L4RE_RM_OVERMAP      = 0x10, /**< \brief Unmap memory already mapped in the region */
  L4RE_RM_SEARCH_ADDR  = 0x20, /**< \brief Search for a suitable address range */
  L4RE_RM_IN_AREA      = 0x40, /**< \brief Search only in area, or map into area */
  L4RE_RM_EAGER_MAP    = 0x80, /**< \brief Eagerly map the attached data space in. */
  L4RE_RM_ATTACH_FLAGS = 0xf0, /**< \brief Mask of all attach flags */
};



/**
 * \ingroup api_l4re_c_rm
 * \return 0 on success, <0 on error
 * \see L4Re::Rm::reserve_area
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_reserve_area(l4_addr_t *start, unsigned long size,
                     unsigned flags, unsigned char align) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \return 0 on success, <0 on error
 * \see L4Re::Rm::free_area
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_free_area(l4_addr_t addr) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \copydetails L4Re::Rm::attach
 * \return 0 on success, <0 on error
 * \see L4Re::Rm::attach
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_attach(void **start, unsigned long size, unsigned long flags,
               l4re_ds_t const mem, l4_addr_t offs,
               unsigned char align) L4_NOTHROW;


/**
 * \ingroup api_l4re_c_rm
 * \brief Detach and unmap in current task.
 * \param addr   Address of the region to detach.
 * \return 0 on success, <0 on error
 *
 * Also \see L4Re::Rm::detach
 *
 * This function is using the L4::Env::env()->rm() service.
 */
L4_CV L4_INLINE int
l4re_rm_detach(void *addr) L4_NOTHROW;

/**
 * \ingroup api_l4re_c_rm
 * \brief Detach, unmap and return affected dataspace in current task.
 * \param addr   Address of the region to detach.
 * \retval ds    Returns dataspace that is affected.
 *
 * \return 0 on success, <0 on error
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
 * \param addr   Address of the region to detach.
 * \retval ds    Returns dataspace that is affected.
 * \param task   Task to unmap pages from, specify L4_INVALID_CAP to not unmap
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
 * \return 0 on success, <0 on error
 * \see L4Re::Rm::find
 */
L4_CV L4_INLINE int
l4re_rm_find(l4_addr_t *addr, unsigned long *size, l4_addr_t *offset,
             unsigned *flags, l4re_ds_t *m) L4_NOTHROW;

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
                         unsigned flags, unsigned char align) L4_NOTHROW;

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
                   unsigned long flags, l4re_ds_t const mem, l4_addr_t offs,
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
                 unsigned long *size, l4_addr_t *offset,
                 unsigned *flags, l4re_ds_t *m) L4_NOTHROW;

/**
 * \brief Dump region map internal data structures.
 * \ingroup api_l4re_c_rm
 */
L4_CV void
l4re_rm_show_lists_srv(l4_cap_idx_t rm) L4_NOTHROW;


/********** Implementations ***************************/

L4_CV L4_INLINE int
l4re_rm_reserve_area(l4_addr_t *start, unsigned long size,
                     unsigned flags, unsigned char align) L4_NOTHROW
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
l4re_rm_attach(void **start, unsigned long size, unsigned long flags,
               l4re_ds_t const mem, l4_addr_t offs,
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
l4re_rm_find(l4_addr_t *addr, unsigned long *size, l4_addr_t *offset,
             unsigned *flags, l4re_ds_t *m) L4_NOTHROW
{
  return l4re_rm_find_srv(l4re_global_env->rm, addr, size, offset, flags, m);
}

L4_CV L4_INLINE void
l4re_rm_show_lists(void) L4_NOTHROW
{
  l4re_rm_show_lists_srv(l4re_global_env->rm);
}

EXTERN_C_END
