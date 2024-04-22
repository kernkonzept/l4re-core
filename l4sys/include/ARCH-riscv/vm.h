/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Georg Kotheimer <georg.kotheimer@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
#pragma once

#include <l4/sys/vcpu.h>

enum
{
  /// Endianness of excplicit guest memory accesses
  L4_vm_hstatus_vsbe       = 1UL << 5,
  /// Guest Virtual Address
  L4_vm_hstatus_gva        = 1UL << 6,
  /// Supervisor Previous Virtual Privilege
  L4_vm_hstatus_spvp       = 1UL << 8,
  /// Raises virtual instruction exception for guest WFI after time limit.
  L4_vm_hstatus_vtw        = 1UL << 21,

  // Controls the effective XLEN for VS-mode, only available on RV64 and only if
  // supported by the hardware.
  L4_vm_hstatus_vsxl_32    = 1UL,
  L4_vm_hstatus_vsxl_64    = 2UL,
  L4_vm_hstatus_vsxl_128   = 3UL,
  L4_vm_hstatus_vsxl_shift = 32,
};

typedef enum L4_vm_rfnc
{
  L4_vm_rfnc_none            = 0,
  L4_vm_rfnc_fence_i         = 1,
  L4_vm_rfnc_sfence_vma      = 2,
  L4_vm_rfnc_sfence_vma_asid = 3,
} L4_vm_rfnc;

enum
{
  L4_vm_hvip_vssip = 1UL << 2,
  L4_vm_hvip_vstip = 1UL << 6,
  L4_vm_hvip_vseip = 1UL << 10,
};

/**
 * L4 extended vCPU state for RISC-V.
 *
 * Contains the additional RISC-V guest state accompanying the `l4_vcpu_state_t`.
 */
typedef struct l4_vm_state_t
{
  l4_umword_t hedeleg;
  l4_umword_t hideleg;

  l4_umword_t hvip;
  l4_umword_t hip; // read-only
  l4_umword_t hie;

  l4_uint64_t htimedelta;

  l4_umword_t htval;
  l4_umword_t htinst;

  l4_umword_t vsstatus;
  l4_umword_t vstvec;
  l4_umword_t vsscratch;
  l4_umword_t vsepc;
  l4_umword_t vscause;
  l4_umword_t vstval;
  l4_umword_t vsatp;
  l4_uint64_t vstimecmp;

  // Indicates that a hypervisor load/store instruction failed. VMM is
  // responsible for resetting this value before executing a hypervisor/load
  // store instruction.
  l4_uint8_t hlsi_failed;

  l4_uint8_t remote_fence;
  l4_umword_t remote_fence_hart_mask;
  l4_umword_t remote_fence_start_addr;
  l4_umword_t remote_fence_size;
  l4_umword_t remote_fence_asid;
} l4_vm_state_t;


L4_INLINE l4_vm_state_t *
l4_vm_state(l4_vcpu_state_t *vcpu) L4_NOTHROW;

L4_INLINE l4_vm_state_t *
l4_vm_state(l4_vcpu_state_t *vcpu) L4_NOTHROW
{ return (l4_vm_state_t *)((char *)vcpu + 0x400); }
