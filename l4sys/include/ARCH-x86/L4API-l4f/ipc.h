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
 * License: see LICENSE.spdx (in this directory or the directories above)
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
