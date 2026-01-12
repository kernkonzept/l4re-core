/*
 * Copyright (C) 2024-2025 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

/**
 * \file
 * Functionality for dumping kernel information.
 */

/**
 * \defgroup fiasco_dump_api Kernel Information Dump
 * \ingroup api_calls_fiasco
 * Kernel information dumping related functionality.
 *
 * Functions that dump various kernel internal information to the console.
 * Probably only present in kernel debug builds.
 *
 * \includefile{l4/sys/kdump.h}
 */

#include <l4/sys/kdebug.h>

/**
 * Dump kernel memory statistics on console.
 *
 * \retval 0            Success.
 * \retval -L4_ENOSYS   Not implemented by kernel.
 */
L4_INLINE long
fiasco_dump_kmem_stats(void);

L4_INLINE long
fiasco_dump_kmem_stats(void)
{
  enum { DUMP_KMEM_STATS = L4_KDEBUG_GROUP_DUMP + 0x00 };
  return l4_error(__l4_kdebug_op(DUMP_KMEM_STATS));
}
