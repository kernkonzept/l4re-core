/*****************************************************************************/
/**
 * \file
 * \brief  x86 port I/O
 *
 * \date   06/2003
 * \author Frank Mehnert <fm3@os.inf.tu-dresden.de>
 */
/*****************************************************************************/

/*
 * (c) 2003-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef _L4UTIL_PORT_IO_H
#define _L4UTIL_PORT_IO_H

/**
 * \defgroup l4util_portio IA32 Port I/O API
 * \ingroup l4util_api
 */

/* L4 includes */
#include <l4/sys/l4int.h>
#include <l4/sys/compiler.h>

/*****************************************************************************
 *** Prototypes
 *****************************************************************************/

EXTERN_C_BEGIN
/**
 * \addtogroup l4util_portio
 */
/*@{*/
/**
 * \brief Read byte from I/O port
 *
 * \param  port	   I/O port address
 * \return value
 */
L4_INLINE l4_uint8_t
l4util_in8(l4_uint16_t port);

/**
 * \brief Read 16-bit-value from I/O port
 *
 * \param  port	   I/O port address
 * \return value
 */
L4_INLINE l4_uint16_t
l4util_in16(l4_uint16_t port);

/**
 * \brief Read 32-bit-value from I/O port
 *
 * \param  port	   I/O port address
 * \return value
 */
L4_INLINE l4_uint32_t
l4util_in32(l4_uint16_t port);

/**
 * \brief Read a block of 8-bit-values from I/O ports
 *
 * \param  port	   I/O port address
 * \param  addr    address of buffer
 * \param  count   number of I/O operations
 */
L4_INLINE void
l4util_ins8(l4_uint16_t port, l4_umword_t addr, l4_umword_t count);

/**
 * \brief Read a block of 16-bit-values from I/O ports
 *
 * \param  port	   I/O port address
 * \param  addr    address of buffer
 * \param  count   number of I/O operations
 */
L4_INLINE void
l4util_ins16(l4_uint16_t port, l4_umword_t addr, l4_umword_t count);

/**
 * \brief Read a block of 32-bit-values from I/O ports
 *
 * \param  port	   I/O port address
 * \param  addr    address of buffer
 * \param  count   number of I/O operations
 */
L4_INLINE void
l4util_ins32(l4_uint16_t port, l4_umword_t addr, l4_umword_t count);

/**
 * \brief Write byte to I/O port
 *
 * \param  port	   I/O port address
 * \param  value   value to write
 */
L4_INLINE void
l4util_out8(l4_uint8_t value, l4_uint16_t port);

/**
 * \brief Write 16-bit-value to I/O port
 * \ingroup port_io
 *
 * \param  port	   I/O port address
 * \param  value   value to write
 */
L4_INLINE void
l4util_out16(l4_uint16_t value, l4_uint16_t port);

/**
 * \brief Write 32-bit-value to I/O port
 *
 * \param  port	   I/O port address
 * \param  value   value to write
 */
L4_INLINE void
l4util_out32(l4_uint32_t value, l4_uint16_t port);

/**
 * \brief Write a block of bytes to I/O port
 *
 * \param  port	   I/O port address
 * \param  addr    address of buffer
 * \param  count   number of I/O operations
 */
L4_INLINE void
l4util_outs8(l4_uint16_t port, l4_umword_t addr, l4_umword_t count);

/**
 * \brief Write a block of 16-bit-values to I/O port
 * \ingroup port_io
 *
 * \param  port	   I/O port address
 * \param  addr    address of buffer
 * \param  count   number of I/O operations
 */
L4_INLINE void
l4util_outs16(l4_uint16_t port, l4_umword_t addr, l4_umword_t count);

/**
 * \brief Write block of 32-bit-values to I/O port
 *
 * \param  port	   I/O port address
 * \param  addr    address of buffer
 * \param  count   number of I/O operations
 */
L4_INLINE void
l4util_outs32(l4_uint16_t port, l4_umword_t addr, l4_umword_t count);

/**
 * \brief delay I/O port access by writing to port 0x80
 */
L4_INLINE void
l4util_iodelay(void);

/*@}*/

EXTERN_C_END


/*****************************************************************************
 *** Implementation
 *****************************************************************************/

L4_INLINE l4_uint8_t
l4util_in8(l4_uint16_t port)
{
  l4_uint8_t value;
  asm volatile ("inb %w1, %b0" : "=a" (value) : "Nd" (port));
  return value;
}

L4_INLINE l4_uint16_t
l4util_in16(l4_uint16_t port)
{
  l4_uint16_t value;
  asm volatile ("inw %w1, %w0" : "=a" (value) : "Nd" (port));
  return value;
}

L4_INLINE l4_uint32_t
l4util_in32(l4_uint16_t port)
{
  l4_uint32_t value;
  asm volatile ("inl %w1, %0" : "=a" (value) : "Nd" (port));
  return value;
}

L4_INLINE void
l4util_ins8(l4_uint16_t port, l4_umword_t addr, l4_umword_t count)
{
  l4_umword_t dummy1, dummy2;
  asm volatile ("rep insb" : "=D"(dummy1), "=c"(dummy2)
			   : "d" (port), "D" (addr), "c"(count)
			   : "memory");
}

L4_INLINE void
l4util_ins16(l4_uint16_t port, l4_umword_t addr, l4_umword_t count)
{
  l4_umword_t dummy1, dummy2;
  asm volatile ("rep insw" : "=D"(dummy1), "=c"(dummy2)
			   : "d" (port), "D" (addr), "c"(count)
			   : "memory");
}

L4_INLINE void
l4util_ins32(l4_uint16_t port, l4_umword_t addr, l4_umword_t count)
{
  l4_umword_t dummy1, dummy2;
  asm volatile ("rep insl" : "=D"(dummy1), "=c"(dummy2)
			   : "d" (port), "D" (addr), "c"(count)
			   : "memory");
}

L4_INLINE void
l4util_out8(l4_uint8_t value, l4_uint16_t port)
{
  asm volatile ("outb %b0, %w1" : : "a" (value), "Nd" (port));
}

L4_INLINE void
l4util_out16(l4_uint16_t value, l4_uint16_t port)
{
  asm volatile ("outw %w0, %w1" : : "a" (value), "Nd" (port));
}

L4_INLINE void
l4util_out32(l4_uint32_t value, l4_uint16_t port)
{
  asm volatile ("outl %0, %w1" : : "a" (value), "Nd" (port));
}

L4_INLINE void
l4util_outs8(l4_uint16_t port, l4_umword_t addr, l4_umword_t count)
{
  l4_umword_t dummy1, dummy2;
  asm volatile ("rep outsb" : "=S"(dummy1), "=c"(dummy2)
			    : "d" (port), "S" (addr), "c"(count)
			    : "memory");
}

L4_INLINE void
l4util_outs16(l4_uint16_t port, l4_umword_t addr, l4_umword_t count)
{
  l4_umword_t dummy1, dummy2;
  asm volatile ("rep outsw" : "=S"(dummy1), "=c"(dummy2)
			    : "d" (port), "S" (addr), "c"(count)
			    : "memory");
}

L4_INLINE void
l4util_outs32(l4_uint16_t port, l4_umword_t addr, l4_umword_t count)
{
  l4_umword_t dummy1, dummy2;
  asm volatile ("rep outsl" : "=S"(dummy1), "=c"(dummy2)
			    : "d" (port), "S" (addr), "c"(count)
			    : "memory");
}

L4_INLINE void
l4util_iodelay(void)
{
  asm volatile ("outb %al,$0x80");
}

#endif
