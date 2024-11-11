/**
 * \file
 * \brief   Kumem allocator utility C interface
 */
/*
 * (c) 2010 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *          Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

/**
 * \defgroup api_l4re_c_util_kumem_alloc Kumem allocator utility
 * \ingroup api_l4re_c
 * \brief Kumem allocator utility C interface.
 */

#include <l4/sys/types.h>
#include <l4/sys/linkage.h>

__BEGIN_DECLS

/**
 * \copydoc L4Re::Util::kumem_alloc()
 */
L4_CV int
l4re_util_kumem_alloc(l4_addr_t *mem, unsigned pages_order,
                      l4_cap_idx_t task, l4_cap_idx_t rm) L4_NOTHROW;

__END_DECLS
