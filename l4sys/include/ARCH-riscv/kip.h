/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include_next <l4/sys/kip.h>

/**
 * Check if platform and kernel support a RISC-V ISA extension.
 *
 * \param kip  Pointer to the kernel info page (KIP).
 * \param ext  Extension to check.
 *
 * \return  1 if the kernel supports the feature, 0 if not.
 *
 * Checks the feature field in the KIP for the given string.
 */
L4_INLINE int
l4_kip_has_isa_ext(l4_kernel_info_t const *kip, L4_riscv_isa_ext ext) L4_NOTHROW
{
  if (ext < 0 || ext >= L4_riscv_isa_ext_max)
    return 0;

  return kip->platform_info.arch.isa_ext[ext / 32] & (1 << (ext % 32));
}
