/*
 * Copyright (C) 2021 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

struct l4_kip_platform_info_arch
{
  l4_uint32_t isa_ext[7];
  l4_uint32_t timebase_frequency;
  l4_uint32_t hart_num;
  l4_uint32_t hart_ids[16];
  l4_umword_t plic_addr;
  l4_uint32_t plic_nr_irqs;
  l4_uint32_t plic_hart_irq_targets[16];
};

typedef enum L4_riscv_isa_ext
{
  // IDs 0-25 are reserved for single-letter RISC-V ISA extensions.
  L4_riscv_isa_ext_a = ('a' - 'a'), // Atomics
  L4_riscv_isa_ext_b = ('b' - 'a'), // Bit Manipulation
  L4_riscv_isa_ext_c = ('c' - 'a'), // Quad-Precision Floating-Point
  L4_riscv_isa_ext_d = ('d' - 'a'), // Double-Precision Floating-Point
  L4_riscv_isa_ext_e = ('e' - 'a'), // Reduced Integer
  L4_riscv_isa_ext_f = ('f' - 'a'), // Single-Precision Floating-Point
  L4_riscv_isa_ext_g = ('g' - 'a'), // General
  L4_riscv_isa_ext_h = ('h' - 'a'), // Hypervisor
  L4_riscv_isa_ext_i = ('i' - 'a'), // Integer
  L4_riscv_isa_ext_m = ('m' - 'a'), // Integer Multiplication and Division
  L4_riscv_isa_ext_p = ('p' - 'a'), // Packed-SIMD Extensions
  L4_riscv_isa_ext_q = ('q' - 'a'), // Quad-Precision Floating-Point
  L4_riscv_isa_ext_v = ('v' - 'a'), // Vector

  // IDs starting from 26 represent multi-letter extensions. The assignment does
  // not follow a defined order, IDs are simply assigned incrementally when new
  // extensions are added.
  L4_riscv_isa_ext_base = 26,

  L4_riscv_isa_ext_sstc = 27, // stimecmp / vstimecmp

  // Maximum number of extensions that can be represented, corresponds to the
  // size of the `l4_kip_platform_info_arch.isa_ext` bitmap.
  L4_riscv_isa_ext_max  = 224,
} L4_riscv_isa_ext;
