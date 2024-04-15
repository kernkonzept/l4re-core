/**
 * \internal
 * \file
 * X86 virtualization interface.
 */
/*
 * (c) 2010-2013 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 */
#pragma once

#include <l4/sys/vcpu.h>

/**
 * \defgroup l4_vm_vmx_api VM API for VMX
 * Virtual machine API for VMX.
 * \ingroup l4_vm_api
 */


/**
 * Exported VMX capability registers.
 * \ingroup l4_vm_vmx_api
 */
enum L4_vm_vmx_caps_regs
{
  L4_VM_VMX_BASIC_REG               = 0,    /**< Basic VMX capabilities */
  L4_VM_VMX_TRUE_PINBASED_CTLS_REG  = 1,    /**< True pin-based control caps */
  L4_VM_VMX_TRUE_PROCBASED_CTLS_REG = 2,    /**< True processor based control caps */
  L4_VM_VMX_TRUE_EXIT_CTLS_REG      = 3,    /**< True exit control caps */
  L4_VM_VMX_TRUE_ENTRY_CTLS_REG     = 4,    /**< True entry control caps */
  L4_VM_VMX_MISC_REG                = 5,    /**< Misc caps */
  L4_VM_VMX_CR0_FIXED0_REG          = 6,    /**< Fixed to 0 bits of CR0 */
  L4_VM_VMX_CR0_FIXED1_REG          = 7,    /**< Fixed to 1 bits of CR0 */
  L4_VM_VMX_CR4_FIXED0_REG          = 8,    /**< Fixed to 0 bits of CR4 */
  L4_VM_VMX_CR4_FIXED1_REG          = 9,    /**< Fixed to 1 bits of CR4 */
  L4_VM_VMX_VMCS_ENUM_REG           = 0xa,  /**< VMCS enumeration info */
  L4_VM_VMX_PROCBASED_CTLS2_REG     = 0xb,  /**< Processor based control 2 caps */
  L4_VM_VMX_EPT_VPID_CAP_REG        = 0xc,  /**< EPT and VPID caps */
  L4_VM_VMX_NUM_CAPS_REGS                   /**< Total number of VMX capability registers */
};


/**
 * Exported VMX capability registers (default to 1 bits).
 * \ingroup l4_vm_vmx_api
 */
enum L4_vm_vmx_dfl1_regs
{
  L4_VM_VMX_PINBASED_CTLS_DFL1_REG  = 0x1, /**< Default 1 bits in pin-based controls */
  L4_VM_VMX_PROCBASED_CTLS_DFL1_REG = 0x2, /**< Default 1 bits in processor-based controls */
  L4_VM_VMX_EXIT_CTLS_DFL1_REG      = 0x3, /**< Default 1 bits in exit controls */
  L4_VM_VMX_ENTRY_CTLS_DFL1_REG     = 0x4, /**< Default 1 bits in entry controls */
  L4_VM_VMX_NUM_DFL1_REGS                  /**< Total number of default on registers */
};

/**
 * Get a capability register for VMX.
 * \ingroup l4_vm_vmx_api
 *
 * \param vcpu_state  Pointer to the VCPU state of the VCPU.
 * \param cap_msr     Caps register index (see #L4_vm_vmx_caps_regs).
 * \return The value of the capability register.
 */
L4_INLINE
l4_uint64_t
l4_vm_vmx_get_caps(void const *vcpu_state, unsigned cap_msr) L4_NOTHROW;

/**
 * Get a default to one capability register for VMX.
 * \ingroup l4_vm_vmx_api
 *
 * \param vcpu_state  Pointer to the VCPU state of the VCPU.
 * \param cap_msr     Default 1 caps register index (see #L4_vm_vmx_dfl1_regs).
 * \return The value of the capability register.
 */
L4_INLINE
l4_uint32_t
l4_vm_vmx_get_caps_default1(void const *vcpu_state, unsigned cap_msr) L4_NOTHROW;


/**
 * Additional (virtual) VMCS fields.
 *
 * The VMCS offsets defined here are actually not in the hardware VMCS. However
 * our VMMs run in user mode and need to have access to certain registers
 * available in kernel mode only. So we put them into our version of the VMCS.
 *
 * \ingroup l4_vm_vmx_api
 */
enum L4_vm_vmx_sw_fields
{
  /**
   * VMCS offset for CR2.
   *
   * \note You usually need to check this value against the value you get from
   *       l4_vm_vmx_get_cr2_index() to make sure you are running on a
   *       compatible kernel.
   */
  L4_VM_VMX_VMCS_CR2                = 0x683e,
  /// VMCS offset of extended control register XCR0
  L4_VM_VMX_VMCS_XCR0               = 0x2840,
  /// VMCS offset of system call flag mask MSR
  L4_VM_VMX_VMCS_MSR_SYSCALL_MASK   = 0x2842,
  /// VMCS offset of IA32e mode system call target address MSR
  L4_VM_VMX_VMCS_MSR_LSTAR          = 0x2844,
  /// VMCS offset of IA32 mode system call target address MSR
  L4_VM_VMX_VMCS_MSR_CSTAR          = 0x2846,
  /// VMCS offset of auxiliary TSC signature MSR
  L4_VM_VMX_VMCS_MSR_TSC_AUX        = 0x2848,
  /// VMCS offset of system call target address MSR
  L4_VM_VMX_VMCS_MSR_STAR           = 0x284a,
  /// VMCS offset of GS base address swap target MSR
  L4_VM_VMX_VMCS_MSR_KERNEL_GS_BASE = 0x284c,
};

/**
 * Software VMCS field offset table. The memory layout is as follows:
 *
 * 0x00 - 0x02: 3 offsets for 16-bit fields.
 *        0x03: Reserved.
 * 0x04 - 0x06: 3 offsets for 64-bit fields.
 *        0x07: Reserved.
 * 0x08 - 0x0a: 3 offsets for 32-bit fields.
 *        0x0b: Reserved.
 * 0x0c - 0x0e: 3 offsets for natural-width fields.
 *        0x0f: Reserved.
 * 0x10 - 0x12: 3 limits for 16-bit fields.
 *        0x13: Reserved.
 * 0x14 - 0x16: 3 limits for 64-bit fields.
 *        0x17: Reserved.
 * 0x18 - 0x1a: 3 limits for 32-bit fields.
 *        0x1b: Reserved.
 * 0x1c - 0x1e: 3 limits for natural-width fields.
 *        0x1f: Reserved.
 * 0x20 - 0x23: 4 index shifts.
 *        0x24: Offset of the first software VMCS field.
 *        0x25: Size of the software VMCS fields.
 * 0x26 - 0x27: Reserved.
 *
 * The offsets/limits in each size category are in the following order:
 *  - Control fields.
 *  - Read-only fields.
 *  - Guest fields.
 *
 * The index shifts are in the following order:
 *  - 16-bit.
 *  - 64-bit.
 *  - 32-bit.
 *  - Natural-width.
 *
 * All offsets/limits/sizes are represented in a 64-byte granule.
 *
 * The offsets (after being multiplied by 64) are indexes in the values array
 * in \ref l4_ext_vcpu_state_vmx_t and bit indexes in the dirty_bitmap array in
 * \ref l4_ext_vcpu_state_vmx_t.
 *
 * The limits (after being multiplied by 64) represent the range of the
 * available indexes.
 *
 * \ingroup l4_vm_vmx_api
 */
typedef struct l4_vmx_offset_table_t
{
  l4_uint8_t offsets[4][4];
  l4_uint8_t limits[4][4];
  l4_uint8_t index_shifts[4];
  l4_uint8_t base_offset;
  l4_uint8_t size;

  l4_uint8_t reserved[2];
} l4_vmx_offset_table_t;

enum L4_vm_vmx_vmcs_sizes
{
  L4_VM_VMX_VMCS_SIZE_VALUES = 2560,
  L4_VM_VMX_VMCS_SIZE_DIRTY_BITMAP = 320,
};

/**
 * VMX extended vCPU state.
 *
 * For completeness, this is the overall memory layout of the vCPU:
 *
 * 0x000 - 0x1ff: Standard vCPU state \ref l4_vcpu_state_t (with padding).
 * 0x200 - 0x3ff: VMX capabilities (with padding).
 * 0x400 - 0xfff: VMX extended vCPU state.
 *
 * The memory layout of the VMX extended vCPU state is as follows:
 *
 * 0x000 - 0x007: Reserved (ignored by the kernel). In the hardware VMCS, the
 *                revision identifier and the abort indicator are stored
 *                in this area. Hereby we simply ignore these two entries.
 * 0x008 - 0x00f: User space data (ignored by the kernel). This currently
 *                stores the pointer to a different VMX extended vCPU state
 *                that has been loaded into the given state.
 * 0x010 - 0x013: VMCS field index of the software-defined CR2 field in the
 *                software VMCS.
 * 0x014 - 0x017: Reserved.
 * 0x018 - 0x01f: Capability of the hardware VMCS object (with padding).
 * 0x020 - 0x047: Software VMCS field offset table \ref l4_vmx_offset_table_t.
 * 0x048 - 0x0bf: Reserved.
 * 0x0c0 - 0xabf: Software VMCS fields (with padding).
 * 0xac0 - 0xbff: Software VMCS fields dirty bitmap (with padding).
 *
 * \ingroup l4_vm_vmx_api
 */
typedef struct l4_ext_vcpu_state_vmx_t
{
  l4_uint64_t reserved0;

  l4_uint64_t user_data;
  l4_uint32_t cr2_index;
  l4_uint8_t reserved1[4];

  l4_cap_idx_t vmcs;

  /*
   * Since the capability type size depends on the platform, we add a 32-bit
   * padding if necessary.
   */

#if L4_MWORD_BITS == 32
  l4_uint32_t padding0;
#elif L4_MWORD_BITS == 64
  /* No padding needed. */
#else
  #error Unsupported machine word size.
#endif

  l4_vmx_offset_table_t offset_table;
  l4_uint8_t reserved2[120];

  l4_uint8_t values[L4_VM_VMX_VMCS_SIZE_VALUES];
  l4_uint8_t dirty_bitmap[L4_VM_VMX_VMCS_SIZE_DIRTY_BITMAP];
} l4_ext_vcpu_state_vmx_t;


/**
 * Return length in bytes of a VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param field  Field number.
 * \return Width of field in bytes.
 */
L4_INLINE
unsigned
l4_vm_vmx_field_len(unsigned field) L4_NOTHROW;

/**
 * Return length in power of two (bytes) of a VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param field  Field number.
 * \return Width of field in power of two (bytes).
 */
L4_INLINE
unsigned
l4_vm_vmx_field_order(unsigned field) L4_NOTHROW;

/**
 * Get pointer into software VMCS.
 * \internal
 * \ingroup l4_vm_vmx_api
 *
 * If the given field is not represented in the software VMCS, then NULL is
 * returned.
 *
 * \param vmcs   Pointer to software VMCS buffer.
 * \param field  Field number.
 *
 * \return Pointer to field in the software VMCS or NULL if the field is not
 *         represented in the software VMCS.
 */
L4_INLINE
void *
l4_vm_vmx_field_ptr(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Saves cached state from the kernel software VMCS to the user software VMCS.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs       Pointer to the kernel software VMCS.
 * \param user_vmcs  Pointer to the user software VMCS.
 *
 * This function is comparable to VMX vmclear.
 */
L4_INLINE
void
l4_vm_vmx_clear(void *vmcs, void *user_vmcs) L4_NOTHROW;

/**
 * Loads the user_vmcs as the current software VMCS.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs       Pointer to the kernel software VMCS.
 * \param user_vmcs  Pointer to the user software VMCS.
 *
 * This function is comparable to VMX vmptrld.
 */
L4_INLINE
void
l4_vm_vmx_ptr_load(void *vmcs, void *user_vmcs) L4_NOTHROW;


/**
 * Get the software VMCS field index of the virtual CR2 register.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs  Pointer to the software VMCS.
 * \return The field index used for the virtual CR2 register as used by the
 *         current Fiasco.OC interface.
 *
 * The CR2 register is actually not in the hardware VMCS, however our VMMs run
 * in user mode and need to have access to this register so we put it into our
 * software version of the VMCS.
 *
 * \see #L4_VM_VMX_VMCS_CR2
 */
L4_INLINE
l4_uint32_t
l4_vm_vmx_get_cr2_index(void const *vmcs) L4_NOTHROW;

/**
 * Read a natural-width software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the software VMCS field with the given index.
 */
L4_INLINE
l4_umword_t
l4_vm_vmx_read_nat(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Read a 16-bit software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the software VMCS field with the given index.
 */
L4_INLINE
l4_uint16_t
l4_vm_vmx_read_16(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Read a 32-bit software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the software VMCS field with the given index.
 */
L4_INLINE
l4_uint32_t
l4_vm_vmx_read_32(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Read a 64-bit software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the software VMCS field with the given index.
 */
L4_INLINE
l4_uint64_t
l4_vm_vmx_read_64(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Read any software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the software VMCS field with the given index.
 */
L4_INLINE
l4_uint64_t
l4_vm_vmx_read(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Write to a natural-width software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 * \param val    The value that shall be written to the given field.
 */
L4_INLINE
void
l4_vm_vmx_write_nat(void *vmcs, unsigned field, l4_umword_t val) L4_NOTHROW;

/**
 * Write to a 16-bit software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 * \param val    The value that shall be written to the given field.
 */
L4_INLINE
void
l4_vm_vmx_write_16(void *vmcs, unsigned field, l4_uint16_t val) L4_NOTHROW;

/**
 * Write to a 32-bit software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 * \param val    The value that shall be written to the given field.
 */
L4_INLINE
void
l4_vm_vmx_write_32(void *vmcs, unsigned field, l4_uint32_t val) L4_NOTHROW;

/**
 * Write to a 64-bit software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 * \param val    The value that shall be written to the given field.
 */
L4_INLINE
void
l4_vm_vmx_write_64(void *vmcs, unsigned field, l4_uint64_t val) L4_NOTHROW;

/**
 * Write to an arbitrary software VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 * \param val    The value that shall be written to the given field.
 */
L4_INLINE
void
l4_vm_vmx_write(void *vmcs, unsigned field, l4_uint64_t val) L4_NOTHROW;

/**
 * Associate the software VMCS with a hardware VMCS object capability.
 * \ingroup l4_vm_vmx_api
 *
 * The VMX extended vCPU is unable to run unless it is associated with
 * a hardware VMCS object (i.e. a Vcpu_context object).
 *
 * \note When replacing the hardware VMCS object, the dirty bitmap of the
 *       software VMCS fields is not touched. This is on purpose, to enable
 *       efficient switching between separate VMs. The user is responsible for
 *       explicitly setting those software VMCS bitmap fields that need to be
 *       synchronized to the hardware VMCS.
 *
 * \note The kernel might cache the VMCS object internally (i.e. the capability
 *       is not looked up on every vCPU resume). To remove the association
 *       of the current hardware VMCS object, store an invalid capability with
 *       the bit 3 set.
 *
 * \note If the hardware limitations of the usage of the hardware VMCS are not
 *       observed (i.e. no hardware VMCS being active on more than one physical
 *       CPU), the vCPU will fail to resume.
 *
 * \param vmcs      Pointer to the software VMCS.
 * \param vmcs_cap  Hardware VMCS object capability.
 */
L4_INLINE
void
l4_vm_vmx_set_hw_vmcs(void *vmcs, l4_cap_idx_t vmcs_cap) L4_NOTHROW;

/**
 * Get the hardware VMCS object capability associated with the software VMCS.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs  Pointer to the software VMCS.
 *
 * \return Hardware VMCS object capability.
 */
L4_INLINE
l4_cap_idx_t
l4_vm_vmx_get_hw_vmcs(void *vmcs) L4_NOTHROW;


/* Implementations */

L4_INLINE
unsigned
l4_vm_vmx_field_order(unsigned field) L4_NOTHROW
{
  unsigned size = (field >> 13) & 0x03U;

  switch (size)
    {
    case 0: return 1; /* 16 bits */
    case 1: return 3; /* 64 bits */
    case 2: return 2; /* 32 bits */
    case 3: return (sizeof(l4_umword_t) == 8) ? 3 : 2; /* Natural width */
    }

  __builtin_trap();
}

L4_INLINE
unsigned
l4_vm_vmx_field_len(unsigned field) L4_NOTHROW
{
  return 1 << l4_vm_vmx_field_order(field);
}

L4_INLINE
unsigned
l4_vm_vmx_field_offset(void const *vmcs, unsigned field) L4_NOTHROW
{
  l4_ext_vcpu_state_vmx_t const *state = (l4_ext_vcpu_state_vmx_t const *)vmcs;

  unsigned index = field & 0x3feU;
  unsigned size = (field >> 13) & 0x03U;
  unsigned group = (field >> 10) & 0x03U;

  unsigned shifted_index = index << state->offset_table.index_shifts[size];

  if (shifted_index >= (unsigned)state->offset_table.limits[size][group] * 64)
    return ~0U;

  return (unsigned)state->offset_table.offsets[size][group] * 64
         + shifted_index;
}

L4_INLINE
void *
l4_vm_vmx_field_ptr(void *vmcs, unsigned field) L4_NOTHROW
{
  l4_ext_vcpu_state_vmx_t *state = (l4_ext_vcpu_state_vmx_t *)vmcs;

  unsigned offset = l4_vm_vmx_field_offset(vmcs, field);
  if (offset == ~0U)
    return 0;

  return (void *)(state->values + offset);
}

/**
 * Get pointer and offset into software VMCS..
 * \internal
 * \ingroup l4_vm_vmx_api
 */
L4_INLINE
void *
l4_vm_vmx_field_ptr_offset(void *vmcs, unsigned field, unsigned *offset) L4_NOTHROW
{
  l4_ext_vcpu_state_vmx_t *state = (l4_ext_vcpu_state_vmx_t *)vmcs;

  *offset = l4_vm_vmx_field_offset(vmcs, field);
  if (*offset == ~0U)
    return 0;

  return (void *)(state->values + *offset);
}

/**
 * Set dirty bit in the software VMCS
 * \internal
 * \ingroup l4_vm_vmx_api
 */
L4_INLINE
void
l4_vm_vmx_offset_dirty(void *vmcs, unsigned offset) L4_NOTHROW
{
  l4_ext_vcpu_state_vmx_t *state = (l4_ext_vcpu_state_vmx_t *)vmcs;

  state->dirty_bitmap[offset / 8] |= 1U << (offset % 8);
}

/**
 * Copy full software VMCS state
 * \internal
 */
L4_INLINE
void
l4_vm_vmx_copy_values(l4_ext_vcpu_state_vmx_t const *state, l4_uint8_t *_dst,
                      l4_uint8_t const *_src) L4_NOTHROW
{
  unsigned base_offset = state->offset_table.base_offset * 64;
  unsigned size = state->offset_table.size * 64;

  void *dst = _dst + base_offset;
  void const *src = _src + base_offset;
  __builtin_memcpy(dst, src, size);
}

L4_INLINE
void
l4_vm_vmx_clear(void *vmcs, void *user_vmcs) L4_NOTHROW
{
  l4_ext_vcpu_state_vmx_t *state = (l4_ext_vcpu_state_vmx_t *)vmcs;
  l4_ext_vcpu_state_vmx_t *user_state = (l4_ext_vcpu_state_vmx_t *)user_vmcs;

  void **current_vmcs_ptr = (void **)&state->user_data;
  if (*current_vmcs_ptr != user_vmcs)
    return;

  l4_vm_vmx_set_hw_vmcs(user_state, l4_vm_vmx_get_hw_vmcs(state));
  l4_vm_vmx_copy_values(state, user_state->values, state->values);

  /* Due to its size, the dirty bitmap is always compied in its entirety. */
  __builtin_memcpy(user_state->dirty_bitmap, state->dirty_bitmap,
    L4_VM_VMX_VMCS_SIZE_DIRTY_BITMAP);

  *current_vmcs_ptr = 0;
}

L4_INLINE
void
l4_vm_vmx_ptr_load(void *vmcs, void *user_vmcs) L4_NOTHROW
{
  l4_ext_vcpu_state_vmx_t *state = (l4_ext_vcpu_state_vmx_t *)vmcs;
  l4_ext_vcpu_state_vmx_t *user_state = (l4_ext_vcpu_state_vmx_t *)user_vmcs;

  void **current_vmcs_ptr = (void **)&state->user_data;

  if (*current_vmcs_ptr == user_vmcs)
    return;

  if (*current_vmcs_ptr && *current_vmcs_ptr != user_vmcs)
    l4_vm_vmx_clear(vmcs, *current_vmcs_ptr);

  *current_vmcs_ptr = user_vmcs;

  l4_vm_vmx_set_hw_vmcs(state, l4_vm_vmx_get_hw_vmcs(user_state));
  l4_vm_vmx_copy_values(state, state->values, user_state->values);

  /* Due to its size, the dirty bitmap is always compied in its entirety. */
  __builtin_memcpy(state->dirty_bitmap, user_state->dirty_bitmap,
    L4_VM_VMX_VMCS_SIZE_DIRTY_BITMAP);
}

L4_INLINE
l4_umword_t
l4_vm_vmx_read_nat(void *vmcs, unsigned field) L4_NOTHROW
{
  l4_umword_t *ptr = (l4_umword_t *)l4_vm_vmx_field_ptr(vmcs, field);
  if (!ptr)
    return 0;

  return *ptr;
}

L4_INLINE
l4_uint16_t
l4_vm_vmx_read_16(void *vmcs, unsigned field) L4_NOTHROW
{
  l4_uint16_t *ptr = (l4_uint16_t *)l4_vm_vmx_field_ptr(vmcs, field);
  if (!ptr)
    return 0;

  return *ptr;
}

L4_INLINE
l4_uint32_t
l4_vm_vmx_read_32(void *vmcs, unsigned field) L4_NOTHROW
{
  l4_uint32_t *ptr = (l4_uint32_t *)l4_vm_vmx_field_ptr(vmcs, field);
  if (!ptr)
    return 0;

  return *ptr;
}

L4_INLINE
l4_uint64_t
l4_vm_vmx_read_64(void *vmcs, unsigned field) L4_NOTHROW
{
  l4_uint64_t *ptr = (l4_uint64_t *)l4_vm_vmx_field_ptr(vmcs, field);
  if (!ptr)
    return 0;

  return *ptr;
}

L4_INLINE
l4_uint64_t
l4_vm_vmx_read(void *vmcs, unsigned field) L4_NOTHROW
{
  unsigned size = (field >> 13) & 0x03U;

  switch (size)
    {
    case 0: return l4_vm_vmx_read_16(vmcs, field);
    case 1: return l4_vm_vmx_read_64(vmcs, field);
    case 2: return l4_vm_vmx_read_32(vmcs, field);
    case 3: return l4_vm_vmx_read_nat(vmcs, field);
    }

  __builtin_trap();
}

L4_INLINE
void
l4_vm_vmx_write_nat(void *vmcs, unsigned field, l4_umword_t val) L4_NOTHROW
{
  unsigned offset;
  l4_umword_t *ptr = (l4_umword_t *)l4_vm_vmx_field_ptr_offset(vmcs, field,
                                                               &offset);

  if ((ptr) && (*ptr != val))
    {
      *ptr = val;
      l4_vm_vmx_offset_dirty(vmcs, offset);
    }
}

L4_INLINE
void
l4_vm_vmx_write_16(void *vmcs, unsigned field, l4_uint16_t val) L4_NOTHROW
{
  unsigned offset;
  l4_uint16_t *ptr = (l4_uint16_t *)l4_vm_vmx_field_ptr_offset(vmcs, field,
                                                               &offset);

  if ((ptr) && (*ptr != val))
    {
      *ptr = val;
      l4_vm_vmx_offset_dirty(vmcs, offset);
    }
}

L4_INLINE
void
l4_vm_vmx_write_32(void *vmcs, unsigned field, l4_uint32_t val) L4_NOTHROW
{
  unsigned offset;
  l4_uint32_t *ptr = (l4_uint32_t *)l4_vm_vmx_field_ptr_offset(vmcs, field,
                                                               &offset);

  if ((ptr) && (*ptr != val))
    {
      *ptr = val;
      l4_vm_vmx_offset_dirty(vmcs, offset);
    }
}

L4_INLINE
void
l4_vm_vmx_write_64(void *vmcs, unsigned field, l4_uint64_t val) L4_NOTHROW
{
  unsigned offset;
  l4_uint64_t *ptr = (l4_uint64_t *)l4_vm_vmx_field_ptr_offset(vmcs, field,
                                                               &offset);

  if ((ptr) && (*ptr != val))
    {
      *ptr = val;
      l4_vm_vmx_offset_dirty(vmcs, offset);
    }
}

L4_INLINE
void
l4_vm_vmx_write(void *vmcs, unsigned field, l4_uint64_t val) L4_NOTHROW
{
  unsigned size = (field >> 13) & 0x03U;

  switch (size)
    {
    case 0: l4_vm_vmx_write_16(vmcs, field, val); break;
    case 1: l4_vm_vmx_write_64(vmcs, field, val); break;
    case 2: l4_vm_vmx_write_32(vmcs, field, val); break;
    case 3: l4_vm_vmx_write_nat(vmcs, field, val); break;
    }
}

L4_INLINE
l4_uint64_t
l4_vm_vmx_get_caps(void const *vcpu_state, unsigned cap_msr) L4_NOTHROW
{
  l4_uint64_t const *caps = (l4_uint64_t const *)((char const *)(vcpu_state) + L4_VCPU_OFFSET_EXT_INFOS);
  return caps[cap_msr & 0x0fU];
}

L4_INLINE
l4_uint32_t
l4_vm_vmx_get_caps_default1(void const *vcpu_state, unsigned cap_msr) L4_NOTHROW
{
  l4_uint32_t const *caps = (l4_uint32_t const *)((char const *)(vcpu_state) + L4_VCPU_OFFSET_EXT_INFOS);
  return caps[L4_VM_VMX_NUM_CAPS_REGS * 2 + ((cap_msr & 0x0fU) - L4_VM_VMX_PINBASED_CTLS_DFL1_REG)];
}

L4_INLINE
l4_uint32_t
l4_vm_vmx_get_cr2_index(void const *vmcs) L4_NOTHROW
{
  l4_ext_vcpu_state_vmx_t const *state = (l4_ext_vcpu_state_vmx_t const *)vmcs;
  return state->cr2_index;
}

L4_INLINE
void
l4_vm_vmx_set_hw_vmcs(void *vmcs, l4_cap_idx_t vmcs_cap) L4_NOTHROW
{
  l4_ext_vcpu_state_vmx_t *state = (l4_ext_vcpu_state_vmx_t *)vmcs;
  state->vmcs = vmcs_cap;
}

L4_INLINE
l4_cap_idx_t
l4_vm_vmx_get_hw_vmcs(void *vmcs) L4_NOTHROW
{
  l4_ext_vcpu_state_vmx_t *state = (l4_ext_vcpu_state_vmx_t *)vmcs;
  return state->vmcs & L4_CAP_MASK;
}
