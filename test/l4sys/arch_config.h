/*
 * Copyright (C) 2016 Kernkonzept GmbH.
 * Author(s): Philipp Eppelt <philipp.eppelt@kernkonzept.com>
 *
 * This file is distributed under the terms of the GNU General Public
 * License, version 2. Please see the COPYING-GPL-2 file for details.
 */

/**
 * Architecture specific values to enable architecture independent tests.
 */

#pragma once

#include <l4/sys/types.h>
#include <l4/sys/err.h>

#if defined(ARCH_amd64)
enum Arch_config : l4_umword_t
{
  Max_ipc_label         = 0xfffffffffffffffcUL, ///< largest allowed label for an IPC
  Invalid_cap_error     = L4_ENOMEM,            ///< error code for passing an invalid capability index
  Kernel_memory_address = 0xffffffff83dc3000UL, ///< a memory address in kernel space
};

#else //if defined(ARCH_x86)
enum Arch_config : l4_umword_t
{
  Max_ipc_label         = 0xfffffffcUL, ///< largest allowed label for an IPC
  Invalid_cap_error     = L4_EOK,       ///< error code for passing an invalid capability index
  Kernel_memory_address = 0xf00a8000,   ///< a memory address in kernel space
};
#endif
