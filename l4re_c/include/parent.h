/*
 * Copyright (C) 2024 Kernkonzept GmbH.
 * Author(s): Marcus Haehnel <marcus.haehnel@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

/**
 * \file
 * \brief Parent C interface.
 */

#pragma once

/**
 * \defgroup api_l4re_c_parent Parent interface
 * \ingroup api_l4re_c
 */

#include <l4/sys/compiler.h>
#include <l4/sys/types.h>

L4_BEGIN_DECLS

/**
 * \brief Send a signal using the parent protocol.
 *
 * \param parent  Capability index of parent to send signal to.
 * \param sig     Signal to send
 * \param val     Value of the signal
 *
 * \retval 0  Success
 * \retval <0 IPC error
 *
 * \see L4Re::Parent::signal
 */
L4_CV long L4_EXPORT
l4re_parent_signal(l4_cap_idx_t parent, unsigned long sig, unsigned long val);

L4_END_DECLS
