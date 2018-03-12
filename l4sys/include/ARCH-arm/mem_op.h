/**
 * \file
 * \brief  Memory access functions (ARM specific)
 *
 * \date   2010-10
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2010 Author(s)
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
#ifndef __L4SYS__INCLUDE__ARCH_ARM__MEM_OP_H__
#define __L4SYS__INCLUDE__ARCH_ARM__MEM_OP_H__

#include <l4/sys/compiler.h>
#include <l4/sys/syscall_defs.h>

EXTERN_C_BEGIN

/**
 * \defgroup l4_mem_op_api Memory operations.
 * \ingroup l4_api
 * \brief Operations for memory access.
 *
 * This modules provides functionality to access user task memory from the
 * kernel. This is needed for some devices that are only accessible from
 * privileged processor mode. Only use this when absolutely required. This
 * functionality is only available on the ARM architecture.
 *
 * <c>\#include <l4/sys/mem_op.h></c>
 */

/**
 * \brief Memory access width definitions.
 * \ingroup l4_mem_op_api
 */
enum L4_mem_op_widths
{
  L4_MEM_WIDTH_1BYTE = 0, ///< Access one byte (8-bit width)
  L4_MEM_WIDTH_2BYTE = 1, ///< Access two bytes (16-bit width)
  L4_MEM_WIDTH_4BYTE = 2, ///< Access four bytes (32-bit width)
};

/**
 * \brief Read memory from kernel privilege level.
 * \ingroup l4_mem_op_api
 *
 * \param virtaddress  Virtual address in the calling task.
 * \param width        Width of access in bytes in log2,
 *                       \see L4_mem_op_widths
 * \return Read value.
 *
 * Upon an given invalid address or invalid width value the function does
 * nothing.
 */
L4_INLINE unsigned long
l4_mem_read(unsigned long virtaddress, unsigned width);

/**
 * \brief Write memory from kernel privilege level.
 * \ingroup l4_mem_op_api
 *
 * \param virtaddress  Virtual address in the calling task.
 * \param width        Width of access in bytes in log2
 *                       (i.e. allowed values: 0, 1, 2)
 * \param value        Value to write.
 *
 * Upon an given invalid address or invalid width value the function does
 * nothing.
 */
L4_INLINE void
l4_mem_write(unsigned long virtaddress, unsigned width,
             unsigned long value);

enum L4_mem_ops
{
  L4_MEM_OP_MEM_READ  = 0x10,
  L4_MEM_OP_MEM_WRITE = 0x11,
};

/**
 * \internal
 */
L4_INLINE unsigned long
l4_mem_arm_op_call(unsigned long op,
                   unsigned long va,
                   unsigned long width,
                   unsigned long value);

/** Implementations */

L4_INLINE unsigned long
l4_mem_arm_op_call(unsigned long op,
                   unsigned long va,
                   unsigned long width,
                   unsigned long value)
{
  register unsigned long _op    __asm__ ("r0") = op;
  register unsigned long _va    __asm__ ("r1") = va;
  register unsigned long _width __asm__ ("r2") = width;
  register unsigned long _value __asm__ ("r3") = value;

  __asm__ __volatile__
    ("@ l4_cache_op_arm_call(start) \n\t"
     "mov     lr, pc	            \n\t"
     "mov     pc, %[sc]	            \n\t"
     "@ l4_cache_op_arm_call(end)   \n\t"
       :
	"=r" (_op),
	"=r" (_va),
	"=r" (_width),
        "=r" (_value)
       :
       [sc] "i" (L4_SYSCALL_MEM_OP),
	"0" (_op),
	"1" (_va),
	"2" (_width),
        "3" (_value)
       :
	"cc", "memory", "lr"
       );

  return _value;
}

L4_INLINE unsigned long
l4_mem_read(unsigned long virtaddress, unsigned width)
{
  return l4_mem_arm_op_call(L4_MEM_OP_MEM_READ, virtaddress, width, 0);
}

L4_INLINE void
l4_mem_write(unsigned long virtaddress, unsigned width,
             unsigned long value)
{
  l4_mem_arm_op_call(L4_MEM_OP_MEM_WRITE, virtaddress, width, value);
}

EXTERN_C_END

#endif /* ! __L4SYS__INCLUDE__ARCH_ARM__MEM_OP_H__ */
