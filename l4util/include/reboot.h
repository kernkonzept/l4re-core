/**
 * \file
 * \brief Machine restarting functions.
 */
/*
 * (c) 2000-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Norman Feske <nf2@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
/*****************************************************************************/
#ifndef _L4UTIL_REBOOT_H
#define _L4UTIL_REBOOT_H

#include <l4/sys/compiler.h>

/**
 * \defgroup l4util_reboot Machine Restarting Function
 * \ingroup l4util_api
 */

/**
 * \brief Machine reboot
 * \ingroup l4util_reboot
 */
EXTERN_C_BEGIN
L4_CV void l4util_reboot(void) __attribute__ ((__noreturn__));
EXTERN_C_END

#endif /* ! _L4UTIL_REBOOT_H */
