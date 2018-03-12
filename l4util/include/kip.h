/**
 * \file
 * \ingroup l4util_kip_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#pragma once

#include <l4/sys/kip.h>
#include <l4/sys/compiler.h>

/**
 * \defgroup l4util_kip_api Kernel Interface Page API
 * \ingroup l4util_api
 */
/*@{*/


EXTERN_C_BEGIN

/**
 * \brief Return whether the kernel is running native or under UX.
 *
 * Returns whether the kernel is running natively or under UX. The KIP will
 * be mapped if not already mapped. The KIP will not be unmapped again.
 *
 * \return 1 when running under UX, 0 if not running under UX
 */
L4_CV int l4util_kip_kernel_is_ux(l4_kernel_info_t *);

/**
 * \brief Check if kernel supports a feature.
 *
 * \param str   Feature name to check.
 *
 * \return 1 if the kernel supports the feature, 0 if not.
 *
 * Checks the feature field in the KIP for the given string. The KIP will be
 * mapped if not already mapped. The KIP will not be unmapped again.
 */
L4_CV int l4util_kip_kernel_has_feature(l4_kernel_info_t *, const char *str);

/**
 * \brief Return kernel ABI version.
 *
 * \return Kernel ABI version.
 */
L4_CV unsigned long l4util_kip_kernel_abi_version(l4_kernel_info_t *);

/**
 * \brief Return end of virtual memory.
 * \ingroup l4util_memdesc
 *
 * \return 0 if memory descriptor could not be found,
 *         last address of address space otherwise
 */
L4_CV l4_addr_t l4util_memdesc_vm_high(l4_kernel_info_t *kinfo);

EXTERN_C_END

/**
 * \brief Cycle through kernel features given in the KIP.
 *
 * Cycles through all KIP kernel feature strings. s must be a character
 * pointer (char *) initialized with l4util_kip_version_string().
 */
#define l4util_kip_for_each_feature(s)				\
		for (s += strlen(s) + 1; *s; s += strlen(s) + 1)

/*@}*/

