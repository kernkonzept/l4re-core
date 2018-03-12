/**
 * \file
 * \brief   L4 IPC System Calls, x86
 * \ingroup l4_api
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Alexander Warg <warg@os.inf.tu-dresden.de>,
 *               Lars Reuther <reuther@os.inf.tu-dresden.de>
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
#ifndef __L4_IPC_H__
#define __L4_IPC_H__

#include <l4/sys/types.h>

#include_next <l4/sys/ipc.h>

/*****************************************************************************
 *** Implementation
 *****************************************************************************/

#include <l4/sys/ipc-invoke.h>
#include "ipc-l42-gcc3-nopic.h"

#endif /* !__L4_IPC_H__ */
