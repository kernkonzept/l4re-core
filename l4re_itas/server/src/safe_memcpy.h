/*
 * Copyright (C) 2026 Kernkonzept GmbH.
 * Author(s): Jan Klötzke <jan.kloetzke@kernkonzept.com>
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */

#pragma once

#include <l4/sys/l4int.h>

/**
 * Safely copy buffers.
 *
 * Returns true if the copy succeeded. If a page fault happens while accessing
 * any of the buffers, false is returned.
 */
extern "C" bool safe_memcpy(void *dst, void const *src, l4_size_t size);

extern "C" char safe_memcpy_end[];
extern "C" char safe_memcpy_fault[];
