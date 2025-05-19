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
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#ifndef __L4UTIL__INCLUDE__KPRINTF_H__
#define __L4UTIL__INCLUDE__KPRINTF_H__

#include <l4/sys/compiler.h>

L4_BEGIN_DECLS

L4_CV int l4_kprintf(const char *fmt, ...)
                     __attribute__((format (printf, 1, 2)));

L4_END_DECLS

#endif /* ! __L4UTIL__INCLUDE__KPRINTF_H__ */
