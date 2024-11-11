/*
 * (c) 2013 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

/**
 * \internal
 * Do not expose as any interface yet as this assumes equality over all CPUs
 * which will not be true along the way.
 */
struct l4_kip_platform_info_arch
{
  struct
  {
    l4_uint32_t MIDR, CTR, TCMTR, TLBTR, MPIDR, REVIDR;
    l4_uint32_t ID_PFR[2], ID_DFR0, ID_AFR0, ID_MMFR[4], ID_ISAR[6];
  } cpuinfo;
};
