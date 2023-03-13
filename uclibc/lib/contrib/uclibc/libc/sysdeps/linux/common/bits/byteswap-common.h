/* Macros to swap the order of bytes in integer values.
   Copyright (C) 1997,1998,2000,2001,2002,2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#if !defined _BYTESWAP_H && !defined _NETINET_IN_H
# error "Never use <bits/byteswap.h> directly; include <byteswap.h> instead."
#endif

#ifndef _BITS_BYTESWAP_H
#define _BITS_BYTESWAP_H 1

/* Swap bytes in 16 bit value.  */
#define __bswap_constant_16(x) __builtin_bswap16(x)
#define __bswap_non_constant_16(x) __builtin_bswap16(x)
#define __bswap_16(x) __builtin_bswap16(x)

/* Swap bytes in 32 bit value.  */
#define __bswap_constant_32(x) __builtin_bswap32(x)
#define __bswap_non_constant_32(x) __builtin_bswap32(x)
#define __bswap_32(x) __builtin_bswap32(x)

#if defined __GNUC__ && __GNUC__ >= 2
/* Swap bytes in 64 bit value.  */
# define __bswap_constant_64(x) __builtin_bswap64(x)
# define __bswap_non_constant_64(x) __builtin_bswap64(x)
# define __bswap_64(x) __builtin_bswap64(x)
#endif

#endif /* _BITS_BYTESWAP_H */
