/*****************************************************************************/
/**
 * \file
 * \brief   printf using the kernel debugger
 *
 * \date    04/05/2007
 * \author  Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 */
/*
 * (c) 2007-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#ifndef __L4UTIL__INCLUDE__KPRINTF_H__
#define __L4UTIL__INCLUDE__KPRINTF_H__

#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

L4_CV int l4_kprintf(const char *fmt, ...)
                     __attribute__((format (printf, 1, 2)));

EXTERN_C_END

#endif /* ! __L4UTIL__INCLUDE__KPRINTF_H__ */
