/**
 * \addtogroup l4_basic_types Integer Types
 * \ingroup l4_api
 *
 * \includefile{l4/sys/l4int.h}
 */

/**
 * \file
 * Fixed sized integer types, generic version.
 * \ingroup l4_basic_types
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
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
#ifndef __L4_SYS_L4INT_H__
#define __L4_SYS_L4INT_H__

/* fixed sized data types */
typedef signed char             l4_int8_t;    /**< Signed 8bit value. \ingroup l4_basic_types */
typedef unsigned char           l4_uint8_t;   /**< Unsigned 8bit value. \ingroup l4_basic_types */
typedef signed short int        l4_int16_t;   /**< Signed 16bit value. \ingroup l4_basic_types */
typedef unsigned short int      l4_uint16_t;  /**< Unsigned 16bit value. \ingroup l4_basic_types */
typedef signed int              l4_int32_t;   /**< Signed 32bit value. \ingroup l4_basic_types */
typedef unsigned int            l4_uint32_t;  /**< Unsigned 32bit value. \ingroup l4_basic_types */
typedef signed long long        l4_int64_t;   /**< Signed 64bit value. \ingroup l4_basic_types */
typedef unsigned long long      l4_uint64_t;  /**< Unsigned 64bit value. \ingroup l4_basic_types */

/* some common data types */
typedef unsigned long           l4_addr_t;    /**< Address type \ingroup l4_basic_types */
//do-we-need-this?//typedef unsigned long           l4_offs_t;    /**< Address offset type \ingroup l4_basic_types */


typedef signed long             l4_mword_t;   /**< Signed machine word.
					       **  \ingroup l4_basic_types
					       **/
typedef unsigned long           l4_umword_t;  /**< Unsigned machine word.
					       **  \ingroup l4_basic_types
					       **/
/**
 * CPU clock type.
 * \ingroup l4_basic_types
 */
typedef l4_uint64_t l4_cpu_time_t;

/**
 * Kernel clock type.
 * \ingroup l4_basic_types
 */
typedef l4_uint64_t l4_kernel_clock_t;

#endif /* !__L4_SYS_L4INT_H__ */
