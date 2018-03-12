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
enum
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
 * Get pointer into VMCS.
 * \internal
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to VMCS buffer.
 * \param field  Field number.
 *
 * \return Pointer to field in the VMCS.
 */
L4_INLINE
void *
l4_vm_vmx_field_ptr(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Saves cached state from the kernel VMCS to the user VMCS.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs       Pointer to the kernel VMCS.
 * \param user_vmcs  Pointer to the user VMCS.
 *
 * This function is comparable to VMX vmclear.
 */
L4_INLINE
void
l4_vm_vmx_clear(void *vmcs, void *user_vmcs) L4_NOTHROW;

/**
 * Loads the user_vmcs as the current VMCS.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs       Pointer to the kernel VMCS.
 * \param user_vmcs  Pointer to the user VMCS.
 *
 * This function is comparable to VMX vmptrld.
 */
L4_INLINE
void
l4_vm_vmx_ptr_load(void *vmcs, void *user_vmcs) L4_NOTHROW;


/**
 * Get the VMCS field index of the virtual CR2 register.
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
 * Read a natural width VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the VMCS field with the given index.
 */
L4_INLINE
l4_umword_t
l4_vm_vmx_read_nat(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Read a 16bit VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the VMCS field with the given index.
 */
L4_INLINE
l4_uint16_t
l4_vm_vmx_read_16(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Read a 32bit VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the VMCS field with the given index.
 */
L4_INLINE
l4_uint32_t
l4_vm_vmx_read_32(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Read a 64bit VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the VMCS field with the given index.
 */
L4_INLINE
l4_uint64_t
l4_vm_vmx_read_64(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Read any VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 *
 * \return The value of the VMCS field with the given index.
 */
L4_INLINE
l4_uint64_t
l4_vm_vmx_read(void *vmcs, unsigned field) L4_NOTHROW;

/**
 * Write to a natural width VMCS field.
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
 * Write to a 16bit VMCS field.
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
 * Write to a 32bit VMCS field.
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
 * Write to a 64bit VMCS field.
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
 * Write to an arbitrary VMCS field.
 * \ingroup l4_vm_vmx_api
 *
 * \param vmcs   Pointer to the software VMCS.
 * \param field  The VMCS field index as used on VMX hardware.
 * \param val    The value that shall be written to the given field.
 */
L4_INLINE
void
l4_vm_vmx_write(void *vmcs, unsigned field, l4_uint64_t val) L4_NOTHROW;


/* Implementations */

L4_INLINE
unsigned
l4_vm_vmx_field_order(unsigned field) L4_NOTHROW
{
  switch (field >> 13)
    {
    case 0: return 1;
    case 1: return 3;
    case 2: return 2;
    case 3: if (sizeof(l4_umword_t) == 8) return 3; else return 2;
    default: return 0;
    }
}


L4_INLINE
unsigned
l4_vm_vmx_field_len(unsigned field) L4_NOTHROW
{
  return 1 << l4_vm_vmx_field_order(field);
}


/* Internal VCPU state layout:
 *
 * VCPU State:
 * 0 - xxx: normal IA32 VCPU state (l4_vcpu_state_t)
 *    200h: VMX capabilities (see l4_vm_vmx_get_caps)
 *    400h: Fiasco.OC VMCS
 *
 * Fiasco.OC VMCS:
 *  0h -   7h: Reserved
 *  8h -   Fh: ignored by kernel, stores current VMCS for l4_vm_vmx_clear...
 * 10h -  13h: L4_VM_VMX_VMCS_CR2 value used by the kernel
 * 14h -  1Fh: Reserved
 * 20h -  3Fh: VMCS field offset table
 * 40h - BFFh: Data (VMCS field data)
 *
 * VMCS field offset table:
 *  0h -  2h: 3 offsets for 16bit fields:
 *            0: Control fields, 1: read-only fields, 2: guest state
 *            all offsets in 64byte granules relative to the start of the VMCS
 *        3h: Reserved
 *  4h -  7h: Index shift values for 16bit, 64bit, 32bit, and natural width fields
 *  8h -  Ah: 3 offsets for 64bit fields
 *  Bh -  Fh: Reserved
 * 10h - 12h: 3 offsets for 32bit fields
 * 13h - 17h: Reserved
 * 18h - 1Ah: 3 offsets for natural width fields
 *       1Bh: Reserved
 *       1Ch: Offset of first VMCS field
 *       1Dh: Full size of VMCS fields
 * 1Eh - 1Fh: Reserved
 *
 */

L4_INLINE
unsigned
l4_vm_vmx_field_offset(void const *vmcs, unsigned field) L4_NOTHROW
{
  // the offset table is at 0x20 offset
  enum { Si = 4 };
  l4_uint8_t const *offsets = (l4_uint8_t const *)vmcs;
  offsets += 0x20;
  return (unsigned)offsets[field >> 10] * 64 + ((field & 0x3ff) << offsets[Si + (field >> 13)]);
}

L4_INLINE
void *
l4_vm_vmx_field_ptr(void *vmcs, unsigned field) L4_NOTHROW
{
  return (void *)((char *)vmcs + l4_vm_vmx_field_offset(vmcs, field));
}

/**
 * Copy full vmcs state
 * \internal
 */
L4_INLINE
void
l4_vm_vmx_copy_state(void const *vmcs, void *_dst, void const *_src) L4_NOTHROW
{
  l4_uint8_t const *offsets = (l4_uint8_t const *)vmcs + 0x20;

  unsigned offs = offsets[28] * 64;
  unsigned size = offsets[29] * 64;
  char *const dst = (char*)_dst + offs;
  char const *const src = (char const *)_src + offs;
  __builtin_memcpy(dst, src, size);
}

L4_INLINE
void
l4_vm_vmx_clear(void *vmcs, void *user_vmcs) L4_NOTHROW
{
  void **current_vmcs = (void **)((char *)vmcs + 8);
  if (*current_vmcs != user_vmcs)
    return;

  l4_vm_vmx_copy_state(vmcs, user_vmcs, vmcs);
  *current_vmcs = 0;
}

L4_INLINE
void
l4_vm_vmx_ptr_load(void *vmcs, void *user_vmcs) L4_NOTHROW
{
  void **current_vmcs = (void **)((char *)vmcs + 8);
  if (*current_vmcs == user_vmcs)
    return;

  if (*current_vmcs && *current_vmcs != user_vmcs)
    l4_vm_vmx_clear(vmcs, *current_vmcs);

  *current_vmcs = user_vmcs;
  l4_vm_vmx_copy_state(vmcs, vmcs, user_vmcs);
}


L4_INLINE
l4_umword_t
l4_vm_vmx_read_nat(void *vmcs, unsigned field) L4_NOTHROW
{ return *(l4_umword_t*)(l4_vm_vmx_field_ptr(vmcs, field)); }

L4_INLINE
l4_uint16_t
l4_vm_vmx_read_16(void *vmcs, unsigned field) L4_NOTHROW
{ return *(l4_uint16_t*)(l4_vm_vmx_field_ptr(vmcs, field)); }

L4_INLINE
l4_uint32_t
l4_vm_vmx_read_32(void *vmcs, unsigned field) L4_NOTHROW
{ return *(l4_uint32_t*)(l4_vm_vmx_field_ptr(vmcs, field)); }

L4_INLINE
l4_uint64_t
l4_vm_vmx_read_64(void *vmcs, unsigned field) L4_NOTHROW
{ return *(l4_uint64_t*)(l4_vm_vmx_field_ptr(vmcs, field)); }

L4_INLINE
l4_uint64_t
l4_vm_vmx_read(void *vmcs, unsigned field) L4_NOTHROW
{
  switch(field >> 13)
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
{ *(l4_umword_t*)(l4_vm_vmx_field_ptr(vmcs, field)) = val; }

L4_INLINE
void
l4_vm_vmx_write_16(void *vmcs, unsigned field, l4_uint16_t val) L4_NOTHROW
{ *(l4_uint16_t*)(l4_vm_vmx_field_ptr(vmcs, field)) = val; }

L4_INLINE
void
l4_vm_vmx_write_32(void *vmcs, unsigned field, l4_uint32_t val) L4_NOTHROW
{ *(l4_uint32_t*)(l4_vm_vmx_field_ptr(vmcs, field)) = val; }

L4_INLINE
void
l4_vm_vmx_write_64(void *vmcs, unsigned field, l4_uint64_t val) L4_NOTHROW
{ *(l4_uint64_t*)(l4_vm_vmx_field_ptr(vmcs, field)) = val; }


L4_INLINE
void
l4_vm_vmx_write(void *vmcs, unsigned field, l4_uint64_t val) L4_NOTHROW
{
  switch(field >> 13)
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
  return caps[cap_msr & 0xf];
}

L4_INLINE
l4_uint32_t
l4_vm_vmx_get_caps_default1(void const *vcpu_state, unsigned cap_msr) L4_NOTHROW
{
  l4_uint32_t const *caps = (l4_uint32_t const *)((char const *)(vcpu_state) + L4_VCPU_OFFSET_EXT_INFOS);
  return caps[L4_VM_VMX_NUM_CAPS_REGS * 2 + ((cap_msr & 0xf) - L4_VM_VMX_PINBASED_CTLS_DFL1_REG)];
}

L4_INLINE
l4_uint32_t
l4_vm_vmx_get_cr2_index(void const *vmcs) L4_NOTHROW
{
  l4_uint32_t const *infos = (l4_uint32_t const *)vmcs;
  return infos[3];
}
