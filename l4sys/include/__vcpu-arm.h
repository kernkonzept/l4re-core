/*
 * (c) 2017 Alexander Warg <alexander.warg@kernkonzept.com>
 *
 * This file is part of L4Re and distributed under the terms of the
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

L4_INLINE l4_addr_t l4_vcpu_e_ptr(void const *vcpu, unsigned id) L4_NOTHROW;
L4_INLINE l4_addr_t l4_vcpu_e_ptr(void const *vcpu, unsigned id) L4_NOTHROW
{ return (l4_addr_t)vcpu + 0x400 + (id & 0xfff); }

enum L4_vcpu_e_consts
{
  L4_VCPU_E_NUM_LR = 4, /**< Number of list registers (LRs) */
};

/**
 * Read a 32bit field from the extended vCPU state.
 *
 * \param vcpu  Pointer to the vCPU memory.
 * \param id    Field ID as defined in L4_vcpu_e_field_ids.
 * \returns The value stored in the field.
 */
L4_INLINE l4_uint32_t
l4_vcpu_e_read_32(void const *vcpu, unsigned id) L4_NOTHROW;

L4_INLINE l4_uint32_t
l4_vcpu_e_read_32(void const *vcpu, unsigned id) L4_NOTHROW
{ return *(l4_uint32_t const *)l4_vcpu_e_ptr(vcpu, id); }

/**
 * Write a 32bit field to the extended vCPU state.
 *
 * \param vcpu  Pointer to the vCPU memory.
 * \param id    Field ID as defined in L4_vcpu_e_field_ids.
 * \param val   The value to be written.
 */
L4_INLINE void
l4_vcpu_e_write_32(void *vcpu, unsigned id, l4_uint32_t val) L4_NOTHROW;

L4_INLINE void
l4_vcpu_e_write_32(void *vcpu, unsigned id, l4_uint32_t val) L4_NOTHROW
{ *((l4_uint32_t *)l4_vcpu_e_ptr(vcpu, + id)) = val; }

/**
 * Read a 64bit field from the extended vCPU state.
 *
 * \param vcpu  Pointer to the vCPU memory.
 * \param id    Field ID as defined in L4_vcpu_e_field_ids.
 * \returns The value stored in the field.
 */
L4_INLINE l4_uint64_t
l4_vcpu_e_read_64(void const *vcpu, unsigned id) L4_NOTHROW;

L4_INLINE l4_uint64_t
l4_vcpu_e_read_64(void const *vcpu, unsigned id) L4_NOTHROW
{ return *(l4_uint64_t const *)l4_vcpu_e_ptr(vcpu, id); }

/**
 * Write a 64bit field to the extended vCPU state.
 *
 * \param vcpu  Pointer to the vCPU memory.
 * \param id    Field ID as defined in L4_vcpu_e_field_ids.
 * \param val   The value to be written.
 */
L4_INLINE void
l4_vcpu_e_write_64(void *vcpu, unsigned id, l4_uint64_t val) L4_NOTHROW;

L4_INLINE void
l4_vcpu_e_write_64(void *vcpu, unsigned id, l4_uint64_t val) L4_NOTHROW
{ *((l4_uint64_t *)l4_vcpu_e_ptr(vcpu, id)) = val; }

/**
 * Read a natural register field from the extended vCPU state.
 *
 * \param vcpu  Pointer to the vCPU memory.
 * \param id    Field ID as defined in L4_vcpu_e_field_ids.
 * \returns The value stored in the field.
 */
L4_INLINE l4_umword_t
l4_vcpu_e_read(void const *vcpu, unsigned id) L4_NOTHROW;

L4_INLINE l4_umword_t
l4_vcpu_e_read(void const *vcpu, unsigned id) L4_NOTHROW
{ return *(l4_umword_t const *)l4_vcpu_e_ptr(vcpu, id); }

/**
 * Write a natural register field to the extended vCPU state.
 *
 * \param vcpu  Pointer to the vCPU memory.
 * \param id    Field ID as defined in L4_vcpu_e_field_ids.
 * \param val   The value to be written.
 */
L4_INLINE void
l4_vcpu_e_write(void *vcpu, unsigned id, l4_umword_t val) L4_NOTHROW;

L4_INLINE void
l4_vcpu_e_write(void *vcpu, unsigned id, l4_umword_t val) L4_NOTHROW
{ *((l4_umword_t *)l4_vcpu_e_ptr(vcpu, id)) = val; }
