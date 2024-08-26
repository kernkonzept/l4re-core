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
/**@{*/


__BEGIN_DECLS

/**
 * Check if kernel supports a feature.
 *
 * \param k    Pointer to the kernel info page (KIP).
 * \param str  Feature name to check.
 *
 * \return  1 if the kernel supports the feature, 0 if not.
 *
 * Checks the feature field in the KIP for the given string.
 *
 * \deprecated Use l4_kip_kernel_has_feature().
 */
L4_CV int l4util_kip_kernel_has_feature(l4_kernel_info_t const *k, char const *str);

/**
 * Return kernel ABI version.
 *
 * \param k  Pointer to the kernel info page (KIP).
 * \return  Kernel ABI version.
 */
L4_CV unsigned long l4util_kip_kernel_abi_version(l4_kernel_info_t const *k);

__END_DECLS

/**
 * Cycle through kernel features given in the KIP.
 *
 * Cycles through all KIP kernel feature strings. s must be a character
 * pointer (char const *) initialized with l4_kip_version_string().
 *
 * \deprecated Use l4_kip_for_each_feature().
 */
#define l4util_kip_for_each_feature(s)  l4_kip_for_each_feature(s)

/**@}*/

