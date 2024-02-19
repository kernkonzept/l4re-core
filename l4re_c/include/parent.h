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

#include <l4/sys/types.h>

EXTERN_C_BEGIN

/**
 * \brief Send a signal using the parent protocol.
 * \ingroup api_l4re_c_parent
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

EXTERN_C_END
