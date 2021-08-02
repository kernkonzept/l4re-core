/* SPDX-License-Identifier: ((GPL-2.0-only WITH mif-exception) OR LicenseRef-kk-custom) */
/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 */
#pragma once

struct l4_kip_platform_info_arch
{
  l4_uint32_t isa_ext;
  l4_uint32_t timebase_frequency;
  l4_uint32_t hart_num;
  l4_uint32_t hart_ids[16];
  l4_umword_t plic_addr;
  l4_uint32_t plic_nr_irqs;
  l4_uint32_t plic_hart_irq_targets[16];
};
